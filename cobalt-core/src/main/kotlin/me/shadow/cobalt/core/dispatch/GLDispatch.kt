package me.shadow.cobalt.core.dispatch

/**
 * Neutral GL/EGL dispatch boundary.
 * All provider GL calls go through this boundary, ensuring thread safety
 * and ownership rules are enforced.
 *
 * No provider context may be made current concurrently on two threads
 * unless the adapter explicitly documents and tests that behavior.
 */
interface GLDispatch {

    /**
     * Initializes the GL dispatch table from loaded native libraries.
     *
     * @param glesLibraryPath Path to the GLES library.
     * @param eglLibraryPath Path to the EGL library.
     * @return True if all required function pointers were loaded.
     */
    fun initialize(glesLibraryPath: String, eglLibraryPath: String): Boolean

    /**
     * Returns the dispatch state for diagnostics.
     */
    fun state(): DispatchState

    /**
     * Resets the dispatch table (e.g., on adapter change).
     */
    fun reset()

    /**
     * Returns true if the dispatch is initialized and functional.
     */
    fun isInitialized(): Boolean
}

data class DispatchState(
    val initialized: Boolean,
    val glesVersion: String?,
    val eglVersion: String?,
    val loadedFunctionCount: Int,
    val missingFunctions: List<String>
)
