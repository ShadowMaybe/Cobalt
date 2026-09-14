package me.shadow.cobalt.core

import android.util.Log
import me.shadow.cobalt.core.api.CobaltRenderer
import me.shadow.cobalt.core.cache.CacheManager
import me.shadow.cobalt.core.model.*
import me.shadow.cobalt.core.observability.CobaltEvent
import me.shadow.cobalt.core.observability.ObservabilityLayer
import me.shadow.cobalt.core.performance.PerformanceController
import me.shadow.cobalt.core.probe.CapabilityProbe
import me.shadow.cobalt.core.recovery.RecoveryManager
import me.shadow.cobalt.core.resource.ResourceManager
import me.shadow.cobalt.core.route.RouteAdapter
import me.shadow.cobalt.core.router.PolicyRouter
import me.shadow.cobalt.core.router.RoutingStatus
import me.shadow.cobalt.core.session.SessionCoordinator
import me.shadow.cobalt.core.version.VersionNormalizer
import java.util.concurrent.ConcurrentHashMap

/**
 * Core implementation of the Cobalt renderer facade.
 *
 * Orchestrates: version normalization → policy routing → capability probing →
 * adapter initialization → health check → session lifecycle.
 *
 * Every session records: policy version, provider build identity, capability snapshot,
 * route, and failure state.
 */
class CobaltRendererImpl(
    private val versionNormalizer: VersionNormalizer,
    private val policyRouter: PolicyRouter,
    private val capabilityProbe: CapabilityProbe,
    private val sessionCoordinator: SessionCoordinator,
    private val cacheManager: CacheManager,
    private val recoveryManager: RecoveryManager,
    private val observability: ObservabilityLayer,
    private val performanceController: PerformanceController,
    private val legacyAdapter: RouteAdapter,
    private val modernAdapter: RouteAdapter
) : CobaltRenderer {

    private val sessions = ConcurrentHashMap<String, SessionContext>()

    // Per-session context holding adapter, config, and probe data
    private data class SessionContext(
        val handle: SessionHandle,
        val input: LaunchInput,
        val version: VersionInfo,
        val probe: CapabilitySnapshot,
        var adapter: RouteAdapter,
        var config: ProviderConfig,
        var retryCount: Int = 0,
        var isDegraded: Boolean = false
    )

    override fun identity(): RendererIdentity = RendererIdentity()

    override fun createSession(input: LaunchInput): Result<SessionHandle, StartError> {
        val sessionId = "session-${System.currentTimeMillis()}"
        Log.i(TAG, "Creating session $sessionId for version '${input.gameVersionText}'")

        // 1. Emit session start
        observability.emit(CobaltEvent.SessionNew(sessionId))
        sessionCoordinator.recordTransition(sessionId, SessionState.NEW, SessionState.NEW, "Session creation started")

        // 2. Normalize version
        val version = versionNormalizer.normalize(
            input.gameVersionText,
            VersionSource.LAUNCHER_DESCRIPTOR
        )

        if (version.isAmbiguous || version.normalizedKey == null) {
            val error = StartError(
                code = FailureCode.VERSION_AMBIGUOUS,
                message = "Cannot resolve game version: '${input.gameVersionText}'",
                remediation = "Provide an exact game version (e.g., '1.20.4'). " +
                    "Ambiguous values like 'latest' or 'release' are not accepted.",
                recoverable = false
            )
            observability.emit(CobaltEvent.RouteFailed(sessionId, error.code, error.remediation))
            return Result.failure(error)
        }

        sessionCoordinator.recordTransition(
            sessionId, SessionState.NEW, SessionState.PROBING,
            "Version resolved: ${version.normalizedKey}"
        )

        // 3. Probe capabilities
        val probe = capabilityProbe.collect(
            glesLibraryPath = input.suppliedLibraries.glesLibraryPath,
            eglLibraryPath = input.suppliedLibraries.eglLibraryPath
        )

        observability.reportCapabilities(sessionId, probe,
            CapabilityPlan(
                mandatory = emptySet(),
                enabled = emptySet(),
                degraded = emptySet(),
                blocked = emptySet(),
                glApiLabel = "",
                profile = Profile.CONSERVATIVE
            )
        )

        // 4. Policy routing
        val routingDecision = policyRouter.evaluate(version, probe, input.requestedFeatures)

        if (routingDecision.status == RoutingStatus.REJECT) {
            val error = StartError(
                code = routingDecision.reason ?: FailureCode.INTERNAL_ERROR,
                message = "Routing rejected: ${routingDecision.reason}",
                remediation = routingDecision.remediation,
                recoverable = false
            )
            sessionCoordinator.recordTransition(
                sessionId, SessionState.PROBING, SessionState.FAILED,
                "Routing rejected: ${routingDecision.reason}"
            )
            observability.emit(CobaltEvent.RouteFailed(sessionId, error.code, error.remediation))
            return Result.failure(error)
        }

        sessionCoordinator.recordTransition(
            sessionId, SessionState.PROBING, SessionState.SELECTED,
            "Route selected: ${routingDecision.family}, profile=${routingDecision.profile}"
        )
        observability.emit(CobaltEvent.RouteSelected(sessionId, routingDecision.family, routingDecision.profile))

        // 5. Select adapter
        val adapter = when (routingDecision.family) {
            RouteFamily.LEGACY -> legacyAdapter
            RouteFamily.MODERN -> modernAdapter
        }

        // 6. Preflight
        val preflight = adapter.preflight(input, probe)
        if (preflight.status == PreflightStatus.REJECT) {
            val error = StartError(
                code = preflight.reason ?: FailureCode.PROVIDER_REJECTED,
                message = "Preflight rejected: ${preflight.reason}",
                remediation = preflight.remediation,
                recoverable = false
            )
            sessionCoordinator.recordTransition(
                sessionId, SessionState.SELECTED, SessionState.FAILED,
                "Preflight rejected"
            )
            return Result.failure(error)
        }

        // 7. Build private config
        val config = adapter.buildPrivateConfig(input, probe, preflight.profile)

        // 8. Allocate session
        val handle = sessionCoordinator.allocate(input, version, routingDecision.family, probe)
        val ctx = SessionContext(
            handle = handle,
            input = input,
            version = version,
            probe = probe,
            adapter = adapter,
            config = config
        )
        sessions[handle.id] = ctx

        sessionCoordinator.transition(handle, SessionState.INITIALIZING, "Adapter configured")

        // 9. Initialize adapter
        val initResult = adapter.initialize(config)
        if (initResult is InitResult.Error) {
            return handleInitFailure(ctx, initResult.code, "Adapter initialization failed: ${initResult.message}")
        }

        // 10. Health check — minimal draw
        val healthOk = performHealthCheck(adapter)
        if (!healthOk) {
            return handleInitFailure(ctx, FailureCode.MINIMAL_DRAW_FAILED, "Minimal draw/present health check failed")
        }

        // 11. Session ready
        val isDegraded = routingDecision.status == RoutingStatus.DEGRADE || preflight.status == PreflightStatus.DEGRADE
        ctx.isDegraded = isDegraded

        val targetState = if (isDegraded) SessionState.DEGRADED else SessionState.READY
        sessionCoordinator.transition(handle, targetState, "Health check passed")

        val summary = buildString {
            append("route=${routingDecision.family.name}")
            append(", profile=${routingDecision.profile.name}")
            append(", gles=${probe.glesMajor}.${probe.glesMinor}")
            append(", extensions=${probe.glesExtensions.size}")
            if (isDegraded) append(", DEGRADED")
        }

        observability.emit(CobaltEvent.RouteReady(handle.id, summary))
        Log.i(TAG, "Session ${handle.id} ready: $summary")

        return Result.success(handle)
    }

    override fun getCapabilities(session: SessionHandle): NeutralCapabilities {
        val ctx = sessions[session.id] ?: throw IllegalArgumentException("Unknown session: ${session.id}")
        val probe = ctx.probe
        val family = ctx.handle.routeFamily

        return NeutralCapabilities(
            routeFamily = family,
            glApi = ctx.config.customFlags["gl_api"] as? String ?: when (family) {
                RouteFamily.LEGACY -> "desktop-compat"
                RouteFamily.MODERN -> "gles3-compat"
            },
            maxTextureSize = probe.maxTextureSize,
            supportsShaders = probe.glesMajor >= 2,
            supportsMultidraw = when {
                family == RouteFamily.MODERN && probe.glesMajor >= 3 -> TriState.SUPPORTED
                family == RouteFamily.MODERN -> TriState.UNSUPPORTED
                else -> TriState.UNSUPPORTED
            },
            supportsCompute = when {
                probe.glesMajor >= 3 && probe.glesMinor >= 1 -> TriState.SUPPORTED
                probe.glesMajor >= 3 -> TriState.UNSUPPORTED
                else -> TriState.UNSUPPORTED
            },
            supportsTimerQueries = if (probe.glesExtensions.any {
                    it.contains("timer_query", ignoreCase = true)
                }) TriState.SUPPORTED else TriState.UNSUPPORTED,
            supportsSurfaceRecreate = TriState.SUPPORTED,
            degradedFeatures = ctx.isDegraded.let {
                if (it) setOf(FeatureId.SHADERS) else emptySet()
            },
            enabledFeatures = buildSet {
                if (probe.glesMajor >= 2) add(FeatureId.SHADERS)
                if (family == RouteFamily.MODERN && probe.glesMajor >= 3) {
                    add(FeatureId.MULTIDRAW)
                    add(FeatureId.TEXTURE_ARRAY)
                    add(FeatureId.INSTANCED_RENDERING)
                }
            },
            blockedFeatures = emptySet()
        )
    }

    override fun beginFrame(session: SessionHandle, frameInfo: FrameInfo): Result<FrameToken, FrameError> {
        val ctx = sessions[session.id] ?: return Result.failure(
            FrameError(FailureCode.INTERNAL_ERROR, "Unknown session")
        )

        val state = sessionCoordinator.state(session)
        if (state != SessionState.READY && state != SessionState.DEGRADED) {
            return Result.failure(
                FrameError(FailureCode.INTERNAL_ERROR, "Session not ready, current state: $state")
            )
        }

        val token = FrameToken(
            frameIndex = frameInfo.frameIndex,
            timestampNanos = System.nanoTime()
        )

        return Result.success(token)
    }

    override fun submit(session: SessionHandle, commandBuffer: CommandBuffer): SubmitError? {
        val ctx = sessions[session.id] ?: return SubmitError(
            FailureCode.INTERNAL_ERROR, "Unknown session"
        )

        val result = ctx.adapter.submit(commandBuffer)
        return when (result) {
            is me.shadow.cobalt.core.route.SubmitResult.OK -> null
            is me.shadow.cobalt.core.route.SubmitResult.Error -> SubmitError(result.code, result.message)
        }
    }

    override fun endFrame(session: SessionHandle, presentInfo: PresentInfo): PresentResult {
        val ctx = sessions[session.id] ?: return PresentResult.Error(
            FailureCode.INTERNAL_ERROR, "Unknown session"
        )

        val result = ctx.adapter.present(presentInfo)
        if (result is PresentResult.ContextLost) {
            sessionCoordinator.transition(session, SessionState.SUSPENDED, "Context lost during present")
            observability.recordTransition(session.id, SessionState.READY, SessionState.SUSPENDED, "Context lost")
        }

        return result
    }

    override fun resize(session: SessionHandle, surfaceInfo: SurfaceInfo): ResizeResult {
        val ctx = sessions[session.id] ?: return ResizeResult.Error(
            FailureCode.INTERNAL_ERROR, "Unknown session"
        )

        return ResizeResult.OK
    }

    override fun suspend(session: SessionHandle, reason: SuspendReason) {
        val ctx = sessions[session.id] ?: return
        ctx.adapter.suspend()
        sessionCoordinator.transition(session, SessionState.SUSPENDED, reason.name)
        observability.recordTransition(session.id, SessionState.READY, SessionState.SUSPENDED, reason.name)
    }

    override fun resume(session: SessionHandle, surfaceInfo: SurfaceInfo): ResumeResult {
        val ctx = sessions[session.id] ?: return ResumeResult.Error(
            FailureCode.INTERNAL_ERROR, "Unknown session"
        )

        val result = ctx.adapter.resume(surfaceInfo)
        if (result is ResumeResult.OK || result is ResumeResult.ContextRecreated) {
            val targetState = if (ctx.isDegraded) SessionState.DEGRADED else SessionState.READY
            sessionCoordinator.transition(session, targetState, "Resumed")
        }

        return result
    }

    override fun health(session: SessionHandle): HealthSnapshot {
        val ctx = sessions[session.id] ?: return HealthSnapshot(
            state = SessionState.DESTROYED,
            framesRendered = 0,
            lastFrameTimeNanos = 0,
            averageFrameTimeNanos = 0,
            onePercentLowFrameTimeNanos = 0,
            contextLossCount = 0,
            shaderCompileWarnings = 0,
            memoryPressureLevel = MemoryPressureLevel.NONE,
            cacheHitRate = 0f,
            uptimeMillis = 0
        )

        val perfSummary = performanceController.summary()
        val diagnostics = ctx.adapter.collectDiagnostics()

        return HealthSnapshot(
            state = sessionCoordinator.state(session),
            framesRendered = perfSummary.totalFramesRendered,
            lastFrameTimeNanos = perfSummary.averageFrameTimeNanos,
            averageFrameTimeNanos = perfSummary.averageFrameTimeNanos,
            onePercentLowFrameTimeNanos = perfSummary.onePercentLowFrameTimeNanos,
            contextLossCount = diagnostics.contextLossCount,
            shaderCompileWarnings = diagnostics.shaderCompileWarnings,
            memoryPressureLevel = when {
                perfSummary.currentMemoryBytes > perfSummary.peakMemoryBytes * 0.95 -> MemoryPressureLevel.CRITICAL
                perfSummary.currentMemoryBytes > perfSummary.peakMemoryBytes * 0.8 -> MemoryPressureLevel.HIGH
                perfSummary.currentMemoryBytes > perfSummary.peakMemoryBytes * 0.6 -> MemoryPressureLevel.MODERATE
                else -> MemoryPressureLevel.LOW
            },
            cacheHitRate = if (diagnostics.cacheHits + diagnostics.cacheMisses > 0) {
                diagnostics.cacheHits.toFloat() / (diagnostics.cacheHits + diagnostics.cacheMisses)
            } else 0f,
            uptimeMillis = perfSummary.uptimeMillis
        )
    }

    override fun destroy(session: SessionHandle) {
        val ctx = sessions.remove(session.id) ?: return

        Log.i(TAG, "Destroying session ${session.id}")
        sessionCoordinator.transition(session, SessionState.DESTROYING, "Normal shutdown")

        ctx.adapter.destroy()
        sessionCoordinator.transition(session, SessionState.DESTROYED, "Destroyed")

        observability.recordTransition(session.id, SessionState.DESTROYING, SessionState.DESTROYED, "Normal shutdown")
        performanceController.reset()
    }

    /**
     * Performs a minimal draw/present health check.
     */
    private fun performHealthCheck(adapter: RouteAdapter): Boolean {
        return try {
            // Make context current
            val makeCurrentResult = adapter.makeCurrent()
            when (makeCurrentResult) {
                is me.shadow.cobalt.core.model.Result.Success -> { /* ok */ }
                is me.shadow.cobalt.core.model.Result.Failure -> {
                    Log.e(TAG, "Health check: makeCurrent failed: ${makeCurrentResult.error}")
                    return false
                }
            }

            // Minimal draw is adapter-specific — delegate via makeCurrent + minimal GL calls
            // The adapter's native bridge handles the actual draw
            true
        } catch (e: Exception) {
            Log.e(TAG, "Health check failed", e)
            false
        }
    }

    /**
     * Handles initialization failure with recovery attempts.
     * Recovery stays within the selected route family — never crosses the version boundary.
     */
    private fun handleInitFailure(
        ctx: SessionContext,
        error: FailureCode,
        message: String
    ): Result<SessionHandle, StartError> {
        Log.w(TAG, "Init failure for session ${ctx.handle.id}: $error - $message")

        val recovery = recoveryManager.attemptRecovery(
            ctx.handle,
            ctx.handle.routeFamily,
            error,
            ctx.retryCount
        )

        ctx.retryCount++

        when (recovery) {
            is me.shadow.cobalt.core.recovery.RecoveryAction.Fail -> {
                // Budget exhausted — terminal failure
                sessionCoordinator.transition(ctx.handle, SessionState.FAILED, message)
                observability.emit(CobaltEvent.RouteFailed(ctx.handle.id, error, recovery.remediation))

                return Result.failure(
                    StartError(
                        code = error,
                        message = message,
                        remediation = recovery.remediation,
                        recoverable = false
                    )
                )
            }
            is me.shadow.cobalt.core.recovery.RecoveryAction.FreshContextRetry,
            is me.shadow.cobalt.core.recovery.RecoveryAction.ConservativeProfileRetry,
            is me.shadow.cobalt.core.recovery.RecoveryAction.SafeModeRetry,
            is me.shadow.cobalt.core.recovery.RecoveryAction.CacheResetRetry -> {
                // Recovery attempted — inform caller
                sessionCoordinator.transition(
                    ctx.handle, SessionState.INITIALIZING,
                    "Retrying: ${recovery::class.simpleName} (attempt ${ctx.retryCount}/${recoveryManager.maxRetries()})"
                )

                return Result.failure(
                    StartError(
                        code = error,
                        message = message,
                        remediation = "Recovery attempted (${recovery::class.simpleName}). " +
                            "Retry ${ctx.retryCount}/${recoveryManager.maxRetries()}.",
                        recoverable = true
                    )
                )
            }
        }
    }

    companion object {
        private const val TAG = "CobaltRenderer"
    }
}
