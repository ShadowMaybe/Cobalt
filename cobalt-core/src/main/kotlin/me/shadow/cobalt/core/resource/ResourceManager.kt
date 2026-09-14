package me.shadow.cobalt.core.resource

import me.shadow.cobalt.core.model.CommandBuffer

/**
 * Tracks logical resource identity and lifetime. Owns logical textures, buffers,
 * framebuffers, shaders, programs, and pipeline variants.
 *
 * Provider owns the translation of desktop-style requests into host-compatible forms.
 * Shader conversion is a risk boundary — successful compile does not imply visual correctness.
 */
interface ResourceManager {

    /**
     * Called at the beginning of a frame to prepare resource tracking.
     */
    fun beginFrame(frameIndex: Long)

    /**
     * Called at the end of a frame to finalize resource tracking.
     */
    fun endFrame()

    /**
     * Queues a GL object for deferred deletion on the render thread.
     */
    fun queueDeletion(objectType: ResourceType, objectId: Int)

    /**
     * Processes the deletion queue — must be called on the render thread.
     */
    fun processDeletions()

    /**
     * Returns current resource usage statistics for diagnostics.
     */
    fun statistics(): ResourceStatistics

    /**
     * Invalidates all cached resources (e.g., after cache reset).
     */
    fun invalidateAll()
}

enum class ResourceType {
    TEXTURE,
    BUFFER,
    FRAMEBUFFER,
    RENDERBUFFER,
    SHADER,
    PROGRAM,
    PIPELINE,
    VERTEX_ARRAY
}

data class ResourceStatistics(
    val activeTextures: Int,
    val activeBuffers: Int,
    val activeFramebuffers: Int,
    val activePrograms: Int,
    val pendingDeletions: Int,
    val estimatedGpuMemoryBytes: Long
)
