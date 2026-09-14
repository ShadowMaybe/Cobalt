package me.shadow.cobalt.core.model

/**
 * Errors returned by submit.
 */
data class SubmitError(
    val code: FailureCode,
    val message: String,
    val recoverable: Boolean = false
)
