package me.shadow.cobalt.core.model

/**
 * Structured launch report emitted by the observability layer.
 * All fields are sanitized — no secrets, no unbounded paths.
 */
data class LaunchReport(
    val sessionId: String,
    val policyVersion: String,
    val gameVersionRaw: String,
    val gameVersionKey: VersionKey?,
    val versionSource: VersionSource,
    val routeFamily: RouteFamily?,
    val providerBuildId: String,
    val packageDigest: String?,
    val abi: String,
    val androidApi: Int,
    val eglVendor: String,
    val eglVersion: String,
    val glesVersion: String,
    val glesExtensionsHash: String,
    val surfaceFormat: Int,
    val profileId: Profile,
    val cacheNamespaceHash: String,
    val enabledFeatures: Set<FeatureId>,
    val degradedFeatures: Set<FeatureId>,
    val stateTransitions: List<StateTransition>,
    val failureCode: FailureCode?,
    val retryCount: Int,
    val frameHealthSummary: String?,
    val timestampMillis: Long = System.currentTimeMillis()
)

data class StateTransition(
    val from: SessionState,
    val to: SessionState,
    val timestampMillis: Long,
    val reason: String? = null
)
