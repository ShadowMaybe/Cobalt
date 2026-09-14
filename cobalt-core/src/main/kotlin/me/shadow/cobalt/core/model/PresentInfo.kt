package me.shadow.cobalt.core.model

/**
 * Presentation parameters for endFrame.
 */
data class PresentInfo(
    val swapBuffers: Boolean = true,
    val frameToken: FrameToken
)
