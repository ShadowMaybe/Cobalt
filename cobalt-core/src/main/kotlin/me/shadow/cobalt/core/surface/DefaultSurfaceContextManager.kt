package me.shadow.cobalt.core.surface

import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLContext
import android.opengl.EGLDisplay
import android.opengl.EGLSurface
import android.util.Log
import me.shadow.cobalt.core.model.*

/**
 * Default surface/context manager implementation.
 * Owns EGL display, config, surface, and context creation, binding,
 * resize, suspend, resume, destruction, and teardown.
 *
 * All GL context operations must be performed on the render thread.
 */
class DefaultSurfaceContextManager : SurfaceContextManager {

    private var eglDisplay: EGLDisplay = EGL14.EGL_NO_DISPLAY
    private var eglContext: EGLContext = EGL14.EGL_NO_CONTEXT
    private var eglSurface: EGLSurface = EGL14.EGL_NO_SURFACE
    private var eglConfig: EGLConfig? = null

    private var currentWidth = 0
    private var currentHeight = 0
    private var contextValid = false

    override fun createSurface(
        provider: SurfaceProvider,
        config: ProviderConfig
    ): SurfaceCreationResult {
        Log.i(TAG, "Creating surface: ${provider.width}x${provider.height}")

        try {
            // 1. Get EGL display
            eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)
            if (eglDisplay == EGL14.EGL_NO_DISPLAY) {
                return SurfaceCreationResult.Error(
                    FailureCode.EGL_CONTEXT_UNAVAILABLE,
                    "Failed to get EGL display"
                )
            }

            // 2. Initialize EGL
            val versions = IntArray(2)
            if (!EGL14.eglInitialize(eglDisplay, versions, 0, versions, 1)) {
                return SurfaceCreationResult.Error(
                    FailureCode.EGL_CONTEXT_UNAVAILABLE,
                    "Failed to initialize EGL"
                )
            }

            // 3. Choose config
            val configAttribs = intArrayOf(
                EGL14.EGL_RENDERABLE_TYPE, getRenderableType(config.glesLevel),
                EGL14.EGL_SURFACE_TYPE, EGL14.EGL_WINDOW_BIT,
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                EGL14.EGL_DEPTH_SIZE, 24,
                EGL14.EGL_STENCIL_SIZE, 8,
                EGL14.EGL_NONE
            )

            val configs = arrayOfNulls<EGLConfig>(1)
            val numConfigs = IntArray(1)
            EGL14.eglChooseConfig(eglDisplay, configAttribs, 0, configs, 0, 1, numConfigs, 0)

            if (numConfigs[0] == 0) {
                // Try with reduced requirements
                val fallbackAttribs = intArrayOf(
                    EGL14.EGL_RENDERABLE_TYPE, getRenderableType(config.glesLevel),
                    EGL14.EGL_SURFACE_TYPE, EGL14.EGL_WINDOW_BIT,
                    EGL14.EGL_RED_SIZE, 8,
                    EGL14.EGL_GREEN_SIZE, 8,
                    EGL14.EGL_BLUE_SIZE, 8,
                    EGL14.EGL_NONE
                )
                EGL14.eglChooseConfig(eglDisplay, fallbackAttribs, 0, configs, 0, 1, numConfigs, 0)
            }

            if (numConfigs[0] == 0) {
                return SurfaceCreationResult.Error(
                    FailureCode.EGL_CONTEXT_UNAVAILABLE,
                    "No suitable EGL config found"
                )
            }

            eglConfig = configs[0]

            // 4. Create context
            val contextAttribs = intArrayOf(
                EGL14.EGL_CONTEXT_CLIENT_VERSION, getGlesClientVersion(config.glesLevel),
                EGL14.EGL_NONE
            )

            eglContext = EGL14.eglCreateContext(
                eglDisplay, eglConfig, EGL14.EGL_NO_CONTEXT,
                contextAttribs, 0
            )

            if (eglContext == EGL14.EGL_NO_CONTEXT) {
                return SurfaceCreationResult.Error(
                    FailureCode.CONTEXT_CREATION_FAILED,
                    "Failed to create EGL context"
                )
            }

            // 5. Create window surface
            if (provider.surfaceHandle != 0L) {
                val surfaceAttribs = intArrayOf(EGL14.EGL_NONE)
                eglSurface = EGL14.eglCreateWindowSurface(
                    eglDisplay, eglConfig, provider.surfaceHandle,
                    surfaceAttribs, 0
                )

                if (eglSurface == EGL14.EGL_NO_SURFACE) {
                    return SurfaceCreationResult.Error(
                        FailureCode.SURFACE_RECONFIGURE_FAILED,
                        "Failed to create EGL window surface"
                    )
                }
            }

            // 6. Make context current
            if (!EGL14.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
                return SurfaceCreationResult.Error(
                    FailureCode.CONTEXT_CREATION_FAILED,
                    "Failed to make EGL context current"
                )
            }

            currentWidth = provider.width
            currentHeight = provider.height
            contextValid = true

            Log.i(TAG, "Surface created successfully: ${currentWidth}x${currentHeight}")
            return SurfaceCreationResult.OK(currentWidth, currentHeight)

        } catch (e: Exception) {
            Log.e(TAG, "Surface creation failed", e)
            return SurfaceCreationResult.Error(
                FailureCode.CONTEXT_CREATION_FAILED,
                "Surface creation exception: ${e.message}"
            )
        }
    }

    override fun makeCurrent(): Result<Unit, FailureCode> {
        if (!contextValid) return Result.failure(FailureCode.CONTEXT_CREATION_FAILED)

        return if (EGL14.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            Result.success(Unit)
        } else {
            contextValid = false
            Result.failure(FailureCode.CONTEXT_CREATION_FAILED)
        }
    }

    override fun resize(surfaceInfo: SurfaceInfo): ResizeResult {
        if (!contextValid) {
            return ResizeResult.Error(FailureCode.CONTEXT_CREATION_FAILED, "Context not valid")
        }

        currentWidth = surfaceInfo.width
        currentHeight = surfaceInfo.height

        // On Android, surface resize is handled by destroying and recreating
        // the surface. The actual resize happens through the new surface handle.
        Log.i(TAG, "Surface resize requested: ${currentWidth}x${currentHeight}")
        return ResizeResult.OK
    }

    override fun onSurfaceLost() {
        Log.w(TAG, "Surface lost")
        contextValid = false
        // The surface is invalid but context may still be usable
    }

    override fun recreateSurface(provider: SurfaceProvider): SurfaceCreationResult {
        Log.i(TAG, "Recreating surface: ${provider.width}x${provider.height}")

        // Destroy old surface
        if (eglSurface != EGL14.EGL_NO_SURFACE) {
            EGL14.eglDestroySurface(eglDisplay, eglSurface)
            eglSurface = EGL14.EGL_NO_SURFACE
        }

        // Create new surface with updated provider
        val surfaceAttribs = intArrayOf(EGL14.EGL_NONE)
        eglSurface = EGL14.eglCreateWindowSurface(
            eglDisplay, eglConfig, provider.surfaceHandle,
            surfaceAttribs, 0
        )

        if (eglSurface == EGL14.EGL_NO_SURFACE) {
            return SurfaceCreationResult.Error(
                FailureCode.SURFACE_RECONFIGURE_FAILED,
                "Failed to recreate EGL window surface"
            )
        }

        // Make current with new surface
        if (!EGL14.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            return SurfaceCreationResult.Error(
                FailureCode.CONTEXT_CREATION_FAILED,
                "Failed to make context current after surface recreation"
            )
        }

        currentWidth = provider.width
        currentHeight = provider.height
        contextValid = true

        Log.i(TAG, "Surface recreated: ${currentWidth}x${currentHeight}")
        return SurfaceCreationResult.OK(currentWidth, currentHeight)
    }

    override fun swapBuffers(): Boolean {
        if (!contextValid || eglSurface == EGL14.EGL_NO_SURFACE) return false
        return EGL14.eglSwapBuffers(eglDisplay, eglSurface)
    }

    override fun isContextValid(): Boolean = contextValid

    override fun destroy() {
        Log.i(TAG, "Destroying surface/context")

        // Make no context current first
        if (eglDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglMakeCurrent(
                eglDisplay,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_CONTEXT
            )
        }

        // Destroy surface
        if (eglSurface != EGL14.EGL_NO_SURFACE) {
            EGL14.eglDestroySurface(eglDisplay, eglSurface)
            eglSurface = EGL14.EGL_NO_SURFACE
        }

        // Destroy context
        if (eglContext != EGL14.EGL_NO_CONTEXT) {
            EGL14.eglDestroyContext(eglDisplay, eglContext)
            eglContext = EGL14.EGL_NO_CONTEXT
        }

        // Terminate display
        if (eglDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglTerminate(eglDisplay)
            eglDisplay = EGL14.EGL_NO_DISPLAY
        }

        contextValid = false
        eglConfig = null
        currentWidth = 0
        currentHeight = 0
    }

    private fun getRenderableType(glesLevel: Int): Int {
        return when {
            glesLevel >= 30 -> EGL14.EGL_OPENGL_ES3_BIT
            glesLevel >= 20 -> EGL14.EGL_OPENGL_ES2_BIT
            else -> EGL14.EGL_OPENGL_ES_BIT
        }
    }

    private fun getGlesClientVersion(glesLevel: Int): Int {
        return when {
            glesLevel >= 30 -> 3
            glesLevel >= 20 -> 2
            else -> 1
        }
    }

    companion object {
        private const val TAG = "CobaltSurface"
    }
}
