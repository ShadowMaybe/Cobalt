package me.shadow.cobalt.core.model

/**
 * Private provider configuration — never crosses the public API boundary.
 * Contains library names, environment mappings, shader switches,
 * and launcher-specific fields for the selected route adapter.
 */
data class ProviderConfig(
    val routeFamily: RouteFamily,
    val profile: Profile,
    val libraryPaths: Map<String, String> = emptyMap(),
    val environmentOverrides: Map<String, String> = emptyMap(),
    val shaderProfileHash: String = "",
    val glesLevel: Int = 0,
    val enableShaderCache: Boolean = false,
    val enableProgramCache: Boolean = false,
    val cacheDirectory: String? = null,
    val providerBuildId: String = "",
    val customFlags: Map<String, Any> = emptyMap()
)
