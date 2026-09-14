package me.shadow.cobalt.core.model

/**
 * Opaque command buffer for frame submission.
 * The actual contents are provider-specific; Cobalt tracks ownership only.
 */
data class CommandBuffer(
    val frameIndex: Long,
    val estimatedSizeBytes: Long = 0,
    val containsShaderUpload: Boolean = false,
    val containsTextureUpload: Boolean = false,
    val containsBufferUpload: Boolean = false
)
