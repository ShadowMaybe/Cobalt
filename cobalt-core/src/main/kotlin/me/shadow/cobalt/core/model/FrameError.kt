package me.shadow.cobalt.core.model

/**
 * Errors returned by beginFrame.
 */
data class FrameError(
    val code: FailureCode,
    val message: String,
    val recoverable: Boolean = false
)
