package me.shadow.cobalt.core.model

/**
 * Result of endFrame/present operation.
 */
sealed class PresentResult {
    data object OK : PresentResult()
    data object ContextLost : PresentResult()
    data object PresentFailed : PresentResult()
    data class Error(val code: FailureCode, val message: String) : PresentResult()
}
