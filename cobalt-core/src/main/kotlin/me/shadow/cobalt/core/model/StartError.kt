package me.shadow.cobalt.core.model

/**
 * Errors returned by createSession.
 */
data class StartError(
    val code: FailureCode,
    val message: String,
    val remediation: String? = null,
    val recoverable: Boolean = false
)
