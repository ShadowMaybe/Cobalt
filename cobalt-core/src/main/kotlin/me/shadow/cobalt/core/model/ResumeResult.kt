package me.shadow.cobalt.core.model

/**
 * Result of a session resume operation.
 */
sealed class ResumeResult {
    data object OK : ResumeResult()
    data object ContextRecreated : ResumeResult()
    data class Error(val code: FailureCode, val message: String) : ResumeResult()
}
