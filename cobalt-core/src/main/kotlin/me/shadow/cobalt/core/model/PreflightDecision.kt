package me.shadow.cobalt.core.model

/**
 * Result of a route adapter preflight check.
 */
data class PreflightDecision(
    val status: PreflightStatus,
    val reason: FailureCode? = null,
    val remediation: String? = null,
    val profile: Profile = Profile.CONSERVATIVE
)

enum class PreflightStatus {
    ACCEPT,
    DEGRADE,
    REJECT
}

enum class Profile {
    CONSERVATIVE,
    STANDARD,
    DEGRADED,
    SAFE_MODE
}
