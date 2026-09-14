package me.shadow.cobalt.core.surface

import me.shadow.cobalt.core.model.*

/**
 * Owns display, context, and surface creation, binding, resize, suspend,
 * resume, destruction, recreation, and teardown.
 *
 * Context loss, surface destruction, or resize must not be handled by issuing
 * GL calls from the Android UI thread. Events are posted to the render thread.
 */
interface SurfaceContextManager {

    /**
     * Creates the EGL display, config, surface, and context.
     * Must be called on the render thread.
     */
    fun createSurface(
        provider: SurfaceProvider,
        config: ProviderConfig
    ): SurfaceCreationResult

    /**
     * Makes the context current on the calling thread.
     */
    fun makeCurrent(): Result<Unit, FailureCode>

    /**
     * Resizes the surface to new dimensions.
     * May be called from the event thread; implementation posts to render thread.
     */
    fun resize(surfaceInfo: SurfaceInfo): ResizeResult

    /**
     * Handles surface loss — destroys the current surface and marks
     * the session as needing surface recreation.
     */
    fun onSurfaceLost()

    /**
     * Recreates the surface after loss with a new surface provider.
     * Must be called on the render thread.
     */
    fun recreateSurface(provider: SurfaceProvider): SurfaceCreationResult

    /**
     * Swaps the back buffer (present).
     */
    fun swapBuffers(): Boolean

    /**
     * Queries whether the context is currently valid.
     */
    fun isContextValid(): Boolean

    /**
     * Destroys the surface, context, and display in order.
     * Must be called on the render thread during teardown.
     */
    fun destroy()
}

/**
 * Result of surface creation.
 */
sealed class SurfaceCreationResult {
    data class OK(val width: Int, val height: Int) : SurfaceCreationResult()
    data class Error(val code: FailureCode, val message: String) : SurfaceCreationResult()
}
