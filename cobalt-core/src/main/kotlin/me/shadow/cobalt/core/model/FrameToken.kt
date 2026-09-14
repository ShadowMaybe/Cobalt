package me.shadow.cobalt.core.model

/**
 * Token returned by beginFrame, used to track a frame through submission.
 */
data class FrameToken(
    val frameIndex: Long,
    val timestampNanos: Long
)
