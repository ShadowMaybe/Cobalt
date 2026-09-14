package me.shadow.cobalt.core.observability

import android.util.Log
import me.shadow.cobalt.core.model.*
import java.util.concurrent.ConcurrentLinkedDeque

/**
 * Default observability implementation.
 * Emits structured events, records state transitions, and collects crash bundles.
 * Production default is INFO with sensitive data redacted.
 */
class DefaultObservabilityLayer : ObservabilityLayer {

    private var level: DiagnosticLevel = DiagnosticLevel.INFO
    private val eventRingBuffer = ConcurrentLinkedDeque<CobaltEvent>()
    private val maxRingBufferSize = 1000

    override fun emit(event: CobaltEvent) {
        // Enforce ring buffer size
        while (eventRingBuffer.size >= maxRingBufferSize) {
            eventRingBuffer.pollFirst()
        }
        eventRingBuffer.addLast(event)

        // Log based on level
        when (event) {
            is CobaltEvent.Error -> {
                Log.e(TAG, "[${event.sessionId}] ERROR ${event.code}: ${event.message}")
            }
            is CobaltEvent.RouteFailed -> {
                Log.e(TAG, "[${event.sessionId}] ROUTE_FAILED ${event.code}: ${event.remediation ?: "no remediation"}")
            }
            is CobaltEvent.RouteReady -> {
                Log.i(TAG, "[${event.sessionId}] ROUTE_READY: ${event.summary}")
            }
            is CobaltEvent.RouteSelected -> {
                Log.i(TAG, "[${event.sessionId}] ROUTE_SELECTED: family=${event.family}, profile=${event.profile}")
            }
            is CobaltEvent.SessionNew -> {
                Log.i(TAG, "[${event.sessionId}] SESSION_NEW")
            }
            is CobaltEvent.StateTransition -> {
                Log.i(TAG, "[${event.sessionId}] STATE: ${event.from} -> ${event.to}" +
                    event.reason?.let { " ($it)" }.orEmpty())
            }
            is CobaltEvent.HealthCheck -> {
                if (event.passed) Log.i(TAG, "[${event.sessionId}] HEALTH_OK")
                else Log.w(TAG, "[${event.sessionId}] HEALTH_FAIL: ${event.details}")
            }
            is CobaltEvent.CapabilitySnapshot -> {
                if (level == DiagnosticLevel.DEBUG || level == DiagnosticLevel.TRACE) {
                    Log.d(TAG, "[${event.sessionId}] CAPABILITIES: GLES ${event.snapshot.glesMajor}.${event.snapshot.glesMinor}, " +
                        "extensions=${event.snapshot.glesExtensions.size}, maxTex=${event.snapshot.maxTextureSize}")
                }
            }
            is CobaltEvent.CacheEvent -> {
                if (level == DiagnosticLevel.DEBUG || level == DiagnosticLevel.TRACE) {
                    Log.d(TAG, "[${event.sessionId}] CACHE ${event.action}: ${event.detail ?: ""}")
                }
            }
            is CobaltEvent.ShaderEvent -> {
                if (level == DiagnosticLevel.DEBUG || level == DiagnosticLevel.TRACE) {
                    Log.d(TAG, "[${event.sessionId}] SHADER ${event.shaderId}: ${event.result}")
                }
            }
            is CobaltEvent.PerformanceSample -> {
                if (level == DiagnosticLevel.TRACE) {
                    Log.v(TAG, "[${event.sessionId}] FRAME ${event.frameIndex}: " +
                        "${event.frameTimeNanos / 1_000_000}ms")
                }
            }
        }
    }

    override fun recordTransition(
        sessionId: String,
        from: SessionState,
        to: SessionState,
        reason: String?
    ) {
        emit(CobaltEvent.StateTransition(sessionId, from, to, reason))
    }

    override fun reportCapabilities(
        sessionId: String,
        probe: CapabilitySnapshot,
        plan: CapabilityPlan
    ) {
        emit(CobaltEvent.CapabilitySnapshot(sessionId, probe))
    }

    override fun emitLaunchReport(report: LaunchReport) {
        Log.i(TAG, "Launch report: session=${report.sessionId}, " +
            "version=${report.gameVersionRaw}, family=${report.routeFamily}, " +
            "provider=${report.providerBuildId}, abi=${report.abi}, " +
            "gles=${report.glesVersion}, enabled=${report.enabledFeatures.size}, " +
            "degraded=${report.degradedFeatures.size}, retries=${report.retryCount}" +
            report.failureCode?.let { ", failure=$it" }.orEmpty())
    }

    override fun collectCrashBundle(sessionId: String): CrashBundle {
        val recentEvents = eventRingBuffer.toList().takeLast(100)

        // Redact sensitive data from events
        val sanitizedEvents = recentEvents.map { redactSensitive(it) }

        return CrashBundle(
            sessionId = sessionId,
            launchReport = createEmptyReport(sessionId),
            recentEvents = sanitizedEvents,
            nativeLogTail = collectNativeLogTail(),
            deviceInfo = collectDeviceInfo()
        )
    }

    override fun setLevel(level: DiagnosticLevel) {
        this.level = level
    }

    override fun flush() {
        Log.i(TAG, "Flushing ${eventRingBuffer.size} buffered events")
        // In production, this would write to persistent storage
    }

    private fun redactSensitive(event: CobaltEvent): CobaltEvent {
        // In production, this would redact file paths, shader source, etc.
        return event
    }

    private fun collectNativeLogTail(): String {
        // In production, this would capture recent native logcat output
        return "[native log tail placeholder]"
    }

    private fun collectDeviceInfo(): DeviceInfo {
        return DeviceInfo(
            model = android.os.Build.MODEL,
            manufacturer = android.os.Build.MANUFACTURER,
            androidVersion = android.os.Build.VERSION.RELEASE,
            apiLevel = android.os.Build.VERSION.SDK_INT,
            gpuVendor = "",
            gpuRenderer = "",
            driverVersion = "",
            abi = android.os.Build.SUPPORTED_ABIS.firstOrNull() ?: "unknown",
            totalMemoryMb = Runtime.getRuntime().maxMemory() / (1024 * 1024)
        )
    }

    private fun createEmptyReport(sessionId: String): LaunchReport {
        return LaunchReport(
            sessionId = sessionId,
            policyVersion = "routing-policy-v1",
            gameVersionRaw = "",
            gameVersionKey = null,
            versionSource = VersionSource.NONE,
            routeFamily = null,
            providerBuildId = "",
            packageDigest = null,
            abi = "",
            androidApi = android.os.Build.VERSION.SDK_INT,
            eglVendor = "",
            eglVersion = "",
            glesVersion = "",
            glesExtensionsHash = "",
            surfaceFormat = 0,
            profileId = Profile.CONSERVATIVE,
            cacheNamespaceHash = "",
            enabledFeatures = emptySet(),
            degradedFeatures = emptySet(),
            stateTransitions = emptyList(),
            failureCode = null,
            retryCount = 0,
            frameHealthSummary = null
        )
    }

    companion object {
        private const val TAG = "CobaltObs"
    }
}
