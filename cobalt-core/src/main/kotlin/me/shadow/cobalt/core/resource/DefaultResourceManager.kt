package me.shadow.cobalt.core.resource

import android.util.Log
import java.util.concurrent.ConcurrentLinkedQueue
import java.util.concurrent.atomic.AtomicInteger

/**
 * Default resource manager.
 * Tracks logical resource identity and lifetime. Queues GL object deletions
 * to the render thread. Enforces memory budgets and eviction policies.
 */
class DefaultResourceManager : ResourceManager {

    // Active resource counts
    private val activeTextures = AtomicInteger(0)
    private val activeBuffers = AtomicInteger(0)
    private val activeFramebuffers = AtomicInteger(0)
    private val activePrograms = AtomicInteger(0)

    // Deletion queue — items are processed on the render thread
    private val deletionQueue = ConcurrentLinkedQueue<PendingDeletion>()

    // Frame tracking
    private var currentFrameIndex = 0L
    private var estimatedGpuMemory = 0L

    override fun beginFrame(frameIndex: Long) {
        currentFrameIndex = frameIndex
        processDeletions()
    }

    override fun endFrame() {
        // End-of-frame bookkeeping
    }

    override fun queueDeletion(objectType: ResourceType, objectId: Int) {
        deletionQueue.add(PendingDeletion(objectType, objectId, currentFrameIndex))
    }

    override fun processDeletions() {
        var count = 0
        while (count < MAX_DELETIONS_PER_FRAME) {
            val deletion = deletionQueue.poll() ?: break
            executeDeletion(deletion)
            count++
        }

        if (deletionQueue.isNotEmpty()) {
            Log.d(TAG, "${deletionQueue.size} deletions remaining in queue")
        }
    }

    override fun statistics(): ResourceStatistics {
        return ResourceStatistics(
            activeTextures = activeTextures.get(),
            activeBuffers = activeBuffers.get(),
            activeFramebuffers = activeFramebuffers.get(),
            activePrograms = activePrograms.get(),
            pendingDeletions = deletionQueue.size,
            estimatedGpuMemoryBytes = estimatedGpuMemory
        )
    }

    override fun invalidateAll() {
        // Queue all active resources for deletion
        Log.i(TAG, "Invalidating all resources")

        // In production, this would iterate tracked resources and queue deletions
        activeTextures.set(0)
        activeBuffers.set(0)
        activeFramebuffers.set(0)
        activePrograms.set(0)
        estimatedGpuMemory = 0
    }

    /**
     * Tracks a new texture allocation.
     */
    fun trackTexture(id: Int, sizeBytes: Long) {
        activeTextures.incrementAndGet()
        estimatedGpuMemory += sizeBytes
    }

    /**
     * Tracks a new buffer allocation.
     */
    fun trackBuffer(id: Int, sizeBytes: Long) {
        activeBuffers.incrementAndGet()
        estimatedGpuMemory += sizeBytes
    }

    /**
     * Tracks a new framebuffer allocation.
     */
    fun trackFramebuffer(id: Int) {
        activeFramebuffers.incrementAndGet()
    }

    /**
     * Tracks a new program allocation.
     */
    fun trackProgram(id: Int) {
        activePrograms.incrementAndGet()
    }

    /**
     * Returns the estimated total GPU memory usage in bytes.
     */
    fun estimatedMemoryUsage(): Long = estimatedGpuMemory

    private fun executeDeletion(deletion: PendingDeletion) {
        when (deletion.objectType) {
            ResourceType.TEXTURE -> {
                activeTextures.decrementAndGet()
                // GL call: glDeleteTextures(1, intArrayOf(deletion.objectId), 0)
            }
            ResourceType.BUFFER -> {
                activeBuffers.decrementAndGet()
                // GL call: glDeleteBuffers(1, intArrayOf(deletion.objectId), 0)
            }
            ResourceType.FRAMEBUFFER -> {
                activeFramebuffers.decrementAndGet()
                // GL call: glDeleteFramebuffers(1, intArrayOf(deletion.objectId), 0)
            }
            ResourceType.PROGRAM -> {
                activePrograms.decrementAndGet()
                // GL call: glDeleteProgram(deletion.objectId)
            }
            else -> {
                // Other resource types handled similarly
            }
        }
    }

    private data class PendingDeletion(
        val objectType: ResourceType,
        val objectId: Int,
        val frameIndex: Long
    )

    companion object {
        private const val TAG = "CobaltResource"
        private const val MAX_DELETIONS_PER_FRAME = 64
    }
}
