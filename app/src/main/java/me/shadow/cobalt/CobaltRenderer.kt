package me.shadow.cobalt

import android.view.Surface

/**
 * Cobalt — Unified renderer for Minecraft: Java Edition.
 *
 * Handles both legacy (≤1.17) and modern (>1.17) rendering paths
 * through a single native engine backed by the merged gl4es + MobileGlues core.
 */
class CobaltRenderer {

    companion object {
        init {
            System.loadLibrary("cobalt")
        }

        /** Minecraft version cutoff: versions ≤ this use legacy path */
        const val LEGACY_VERSION_MAJOR = 1
        const val LEGACY_VERSION_MINOR = 17
        const val LEGACY_VERSION_PATCH = 0

        @Volatile
        private var instance: CobaltRenderer? = null

        /**
         * Determine if a Minecraft version should use the legacy rendering path.
         */
        fun isLegacyVersion(major: Int, minor: Int, patch: Int = 0): Boolean {
            return when {
                major < LEGACY_VERSION_MAJOR -> true
                major > LEGACY_VERSION_MAJOR -> false
                minor < LEGACY_VERSION_MINOR -> true
                minor > LEGACY_VERSION_MINOR -> false
                else -> patch <= LEGACY_VERSION_PATCH
            }
        }

        @Synchronized
        fun getInstance(): CobaltRenderer {
            return instance ?: CobaltRenderer().also { instance = it }
        }
    }

    @Volatile
    private var initialized = false

    // ==================== Native methods ====================

    private external fun nativeInit(
        useLegacy: Boolean,
        majorVersion: Int,
        minorVersion: Int
    ): Int

    private external fun nativeSetSurface(surface: Surface?)

    private external fun nativeSetFramebufferSize(width: Int, height: Int)

    private external fun nativeSetEnvironmentVariable(key: String, value: String)

    private external fun nativeSetConfigPath(path: String)

    private external fun nativeShutdown()

    private external fun nativeGetVersion(): String

    private external fun nativeGetRendererName(): String

    // ==================== Public API ====================

    /**
     * Initialize the renderer for a specific Minecraft version.
     *
     * @param majorVersion Minecraft major version (e.g. 1)
     * @param minorVersion Minecraft minor version (e.g. 20)
     * @param patchVersion Minecraft patch version (e.g. 1)
     * @return true if initialization succeeded
     */
    fun init(majorVersion: Int, minorVersion: Int, patchVersion: Int = 0): Boolean {
        if (initialized) {
            shutdown()
        }

        val useLegacy = isLegacyVersion(majorVersion, minorVersion, patchVersion)
        val result = nativeInit(useLegacy, majorVersion, minorVersion)
        initialized = result == 0

        if (initialized) {
            val route = if (useLegacy) "legacy" else "modern"
            android.util.Log.i("Cobalt", "Initialized ($route route) for MC $majorVersion.$minorVersion.$patchVersion")
        } else {
            android.util.Log.e("Cobalt", "Initialization failed with code $result")
        }

        return initialized
    }

    /**
     * Set the rendering surface.
     */
    fun setSurface(surface: Surface?) {
        if (!initialized) return
        nativeSetSurface(surface)
    }

    /**
     * Set the framebuffer dimensions.
     */
    fun setFramebufferSize(width: Int, height: Int) {
        if (!initialized) return
        nativeSetFramebufferSize(width, height)
    }

    /**
     * Set a renderer environment variable (e.g. COBALT_GL, COBALT_ES).
     */
    fun setEnvironmentVariable(key: String, value: String) {
        nativeSetEnvironmentVariable(key, value)
    }

    /**
     * Set the path to the configuration file.
     */
    fun setConfigPath(path: String) {
        nativeSetConfigPath(path)
    }

    /**
     * Apply standard environment variables for Minecraft rendering.
     */
    fun applyMinecraftDefaults(useModern: Boolean) {
        if (useModern) {
            // Modern path: OpenGL ES 3.0+ with GLSL 300 es
            setEnvironmentVariable("COBALT_GL", "31")
            setEnvironmentVariable("COBALT_ES", "3")
            setEnvironmentVariable("COBALT_NORMALIZE", "1")
            setEnvironmentVariable("COBALT_NOERROR", "1")
            setEnvironmentVariable("COBALT_USE_MC_COLOR", "1")
        } else {
            // Legacy path: OpenGL ES 2.0 / GLSL 100
            setEnvironmentVariable("COBALT_GL", "21")
            setEnvironmentVariable("COBALT_ES", "2")
        }
    }

    /**
     * Shut down the renderer and release resources.
     */
    fun shutdown() {
        if (!initialized) return
        nativeShutdown()
        initialized = false
        android.util.Log.i("Cobalt", "Renderer shut down")
    }

    /**
     * Get the renderer version string.
     */
    fun getVersion(): String = nativeGetVersion()

    /**
     * Get the renderer name.
     */
    fun getRendererName(): String = nativeGetRendererName()

    /**
     * Check if the renderer is currently initialized.
     */
    fun isInitialized(): Boolean = initialized
}
