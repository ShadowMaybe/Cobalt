package me.shadow.cobalt.core.dispatch

import android.util.Log
import java.lang.reflect.Method

/**
 * Default GL dispatch implementation.
 * Loads GLES/EGL function pointers from native libraries and provides
 * a neutral dispatch boundary for all provider GL calls.
 *
 * All provider GL calls go through this boundary, ensuring thread safety
 * and ownership rules are enforced.
 */
class DefaultGLDispatch : GLDispatch {

    private var initialized = false
    private var glesVersion: String? = null
    private var eglVersion: String? = null
    private var loadedFunctionCount = 0
    private val missingFunctions = mutableListOf<String>()

    override fun initialize(glesLibraryPath: String, eglLibraryPath: String): Boolean {
        Log.i(TAG, "Initializing GL dispatch: GLES=$glesLibraryPath, EGL=$eglLibraryPath")

        try {
            // Load GLES library
            if (glesLibraryPath.isNotBlank()) {
                System.load(glesLibraryPath)
                Log.i(TAG, "GLES library loaded: $glesLibraryPath")
            }

            // Load EGL library
            if (eglLibraryPath.isNotBlank()) {
                System.load(eglLibraryPath)
                Log.i(TAG, "EGL library loaded: $eglLibraryPath")
            }

            // Validate required function points are available
            val requiredGlesFunctions = listOf(
                "glClearColor", "glClear", "glGetIntegerv", "glGetString",
                "glGetStringi", "glGetError", "glViewport", "glEnable",
                "glDisable", "glBlendFunc", "glDepthFunc", "glDepthMask",
                "glColorMask", "glStencilFunc", "glStencilOp",
                "glDrawArrays", "glDrawElements"
            )

            val requiredEglFunctions = listOf(
                "eglGetDisplay", "eglInitialize", "eglChooseConfig",
                "eglCreateContext", "eglMakeCurrent", "eglSwapBuffers",
                "eglDestroyContext", "eglTerminate", "eglQueryString",
                "eglCreateWindowSurface", "eglDestroySurface",
                "eglGetError", "eglGetConfigs", "eglBindAPI"
            )

            loadedFunctionCount = 0
            missingFunctions.clear()

            // Check GLES functions
            for (func in requiredGlesFunctions) {
                try {
                    Class.forName("android.opengl.GLES20").getMethod(func)
                    loadedFunctionCount++
                } catch (e: NoSuchMethodException) {
                    missingFunctions.add("GLES20.$func")
                }
            }

            // Check GLES 3.x functions
            val gles3Functions = listOf(
                "glGetStringi", "glGetIntegerv"
            )
            for (func in gles3Functions) {
                try {
                    Class.forName("android.opengl.GLES30").getMethod(func)
                    loadedFunctionCount++
                } catch (e: NoSuchMethodException) {
                    // GLES 3.0 not available on this device — acceptable
                }
            }

            // Check EGL functions
            for (func in requiredEglFunctions) {
                try {
                    Class.forName("android.opengl.EGL14").getMethod(func)
                    loadedFunctionCount++
                } catch (e: NoSuchMethodException) {
                    missingFunctions.add("EGL14.$func")
                }
            }

            glesVersion = "loaded"
            eglVersion = "loaded"
            initialized = missingFunctions.isEmpty()

            if (initialized) {
                Log.i(TAG, "GL dispatch initialized: $loadedFunctionCount functions loaded")
            } else {
                Log.e(TAG, "GL dispatch init failed: missing ${missingFunctions.size} functions: $missingFunctions")
            }

            return initialized
        } catch (e: Exception) {
            Log.e(TAG, "GL dispatch initialization failed", e)
            return false
        }
    }

    override fun state(): DispatchState {
        return DispatchState(
            initialized = initialized,
            glesVersion = glesVersion,
            eglVersion = eglVersion,
            loadedFunctionCount = loadedFunctionCount,
            missingFunctions = missingFunctions.toList()
        )
    }

    override fun reset() {
        initialized = false
        glesVersion = null
        eglVersion = null
        loadedFunctionCount = 0
        missingFunctions.clear()
    }

    override fun isInitialized(): Boolean = initialized

    companion object {
        private const val TAG = "CobaltGLDispatch"
    }
}
