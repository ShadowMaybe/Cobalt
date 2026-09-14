package me.shadow.cobalt.core.router

import me.shadow.cobalt.core.model.*

/**
 * Applies the fixed routing policy: <= 1.17.0 → LEGACY, > 1.17.0 → MODERN.
 * Selects one route family and evaluates capabilities against it.
 *
 * The router must never:
 * - Switch across route families during fallback
 * - Guess a version from a GPU, launcher label, or library presence
 * - Accept a successful library load as sufficient health evidence
 */
interface PolicyRouter {

    /**
     * Evaluates the policy for a given version and capability snapshot.
     *
     * @param version The normalized version info.
     * @param probe The collected capability snapshot.
     * @param requestedFeatures Features requested by the launcher/workload.
     * @return A routing decision with the selected family, profile, and any rejections.
     */
    fun evaluate(
        version: VersionInfo,
        probe: CapabilitySnapshot,
        requestedFeatures: Set<FeatureId>
    ): RoutingDecision

    /**
     * Returns the route family that the policy dictates for a given version key.
     * This is a pure function — no side effects, no capability checks.
     */
    fun policyFamily(versionKey: VersionKey): RouteFamily
}

/**
 * Result of the policy router's evaluation.
 */
data class RoutingDecision(
    val family: RouteFamily,
    val status: RoutingStatus,
    val profile: Profile,
    val reason: FailureCode? = null,
    val remediation: String? = null,
    val plan: CapabilityPlan? = null
)

enum class RoutingStatus {
    SELECT,
    DEGRADE,
    REJECT
}
