package me.shadow.cobalt.core.model

/**
 * Public capability report for a session.
 * Contains only neutral identifiers — no provider names or extension strings.
 */
data class NeutralCapabilities(
    val routeFamily: RouteFamily,
    val glApi: String,
    val maxTextureSize: Int,
    val supportsShaders: Boolean,
    val supportsMultidraw: TriState,
    val supportsCompute: TriState,
    val supportsTimerQueries: TriState,
    val supportsSurfaceRecreate: TriState,
    val degradedFeatures: Set<FeatureId>,
    val enabledFeatures: Set<FeatureId>,
    val blockedFeatures: Set<FeatureId>
)
