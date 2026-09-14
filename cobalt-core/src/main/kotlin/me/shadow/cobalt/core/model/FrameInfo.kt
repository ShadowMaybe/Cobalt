package me.shadow.cobalt.core.model

/**
 * Information about the current frame being submitted.
 */
data class FrameInfo(
    val frameIndex: Long,
    val surfaceWidth: Int,
    val surfaceHeight: Int,
    val vsyncEnabled: Boolean = true,
    val isScreenRotated: Boolean = false
)
