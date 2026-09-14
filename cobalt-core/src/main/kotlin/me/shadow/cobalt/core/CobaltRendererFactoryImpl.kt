package me.shadow.cobalt.core

import me.shadow.cobalt.core.api.CobaltRenderer
import me.shadow.cobalt.core.api.CobaltRendererFactory
import me.shadow.cobalt.core.cache.DefaultCacheManager
import me.shadow.cobalt.core.model.RendererIdentity
import me.shadow.cobalt.core.model.RouteFamily
import me.shadow.cobalt.core.observability.DefaultObservabilityLayer
import me.shadow.cobalt.core.performance.DefaultPerformanceController
import me.shadow.cobalt.core.probe.DefaultCapabilityProbe
import me.shadow.cobalt.core.recovery.DefaultRecoveryManager
import me.shadow.cobalt.core.route.RouteAdapter
import me.shadow.cobalt.core.router.DefaultPolicyRouter
import me.shadow.cobalt.core.session.DefaultSessionCoordinator
import me.shadow.cobalt.core.version.SemanticVersionNormalizer

/**
 * Factory implementation that wires together all Cobalt components.
 *
 * In production, adapters would be injected or discovered; here we create
 * them from the legacy and modern modules.
 */
class CobaltRendererFactoryImpl(
    private val legacyAdapter: RouteAdapter? = null,
    private val modernAdapter: RouteAdapter? = null
) : CobaltRendererFactory {

    override fun create(): CobaltRenderer {
        val versionNormalizer = SemanticVersionNormalizer()
        val policyRouter = DefaultPolicyRouter()
        val capabilityProbe = DefaultCapabilityProbe()
        val sessionCoordinator = DefaultSessionCoordinator()
        val cacheManager = DefaultCacheManager()
        val recoveryManager = DefaultRecoveryManager()
        val observability = DefaultObservabilityLayer()
        val performanceController = DefaultPerformanceController()

        val legacy = legacyAdapter ?: createStubAdapter(RouteFamily.LEGACY)
        val modern = modernAdapter ?: createStubAdapter(RouteFamily.MODERN)

        return CobaltRendererImpl(
            versionNormalizer = versionNormalizer,
            policyRouter = policyRouter,
            capabilityProbe = capabilityProbe,
            sessionCoordinator = sessionCoordinator,
            cacheManager = cacheManager,
            recoveryManager = recoveryManager,
            observability = observability,
            performanceController = performanceController,
            legacyAdapter = legacy,
            modernAdapter = modern
        )
    }

    override fun peekIdentity(): RendererIdentity = RendererIdentity()

    /**
     * Creates a stub adapter for testing or when the real adapter module is not linked.
     */
    private fun createStubAdapter(family: RouteFamily): RouteAdapter {
        return StubRouteAdapter(family)
    }
}

/**
 * Minimal stub adapter for unit testing and fallback when the native adapter
 * module is not linked. All methods return success/no-op.
 */
private class StubRouteAdapter(private val family: RouteFamily) : RouteAdapter {
    override fun family(): RouteFamily = family
    override fun preflight(input: me.shadow.cobalt.core.model.LaunchInput, probe: me.shadow.cobalt.core.model.CapabilitySnapshot) =
        me.shadow.cobalt.core.model.PreflightDecision(me.shadow.cobalt.core.model.PreflightStatus.ACCEPT)
    override fun buildPrivateConfig(input: me.shadow.cobalt.core.model.LaunchInput, probe: me.shadow.cobalt.core.model.CapabilitySnapshot, profile: me.shadow.cobalt.core.model.Profile) =
        me.shadow.cobalt.core.model.ProviderConfig(routeFamily = family, profile = profile)
    override fun initialize(config: me.shadow.cobalt.core.model.ProviderConfig) = me.shadow.cobalt.core.model.InitResult.OK
    override fun makeCurrent() = me.shadow.cobalt.core.model.Result.success(Unit)
    override fun translateShader(request: me.shadow.cobalt.core.route.ShaderRequest) =
        me.shadow.cobalt.core.route.ShaderResult.Supported(request.source)
    override fun submit(commandBuffer: me.shadow.cobalt.core.model.CommandBuffer) = me.shadow.cobalt.core.route.SubmitResult.OK
    override fun present(presentInfo: me.shadow.cobalt.core.model.PresentInfo) = me.shadow.cobalt.core.model.PresentResult.OK
    override fun suspend() {}
    override fun resume(surfaceInfo: me.shadow.cobalt.core.model.SurfaceInfo) = me.shadow.cobalt.core.model.ResumeResult.OK
    override fun destroy() {}
    override fun collectDiagnostics() = me.shadow.cobalt.core.model.ProviderDiagnostics(
        routeFamily = family,
        providerBuildId = "stub",
        initializationTimeMs = 0,
        shaderCompilationTimeMs = 0,
        cacheHits = 0,
        cacheMisses = 0,
        shaderCompileWarnings = 0,
        shaderCompileErrors = 0,
        contextLossCount = 0,
        extensionNegotiationLog = emptyList(),
        functionLoaderDecisions = emptyList()
    )
}
