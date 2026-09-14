package me.shadow.cobalt.core.model

/**
 * Diagnostic data collected from a route adapter.
 * Private — used for internal diagnostics and crash reports only.
 */
data class ProviderDiagnostics(
    val routeFamily: RouteFamily,
    val providerBuildId: String,
    val initializationTimeMs: Long,
    val shaderCompilationTimeMs: Long,
    val cacheHits: Int,
    val cacheMisses: Int,
    val shaderCompileWarnings: Int,
    val shaderCompileErrors: Int,
    val contextLossCount: Int,
    val extensionNegotiationLog: List<String>,
    val functionLoaderDecisions: List<String>,
    val customDiagnostics: Map<String, String> = emptyMap()
)
