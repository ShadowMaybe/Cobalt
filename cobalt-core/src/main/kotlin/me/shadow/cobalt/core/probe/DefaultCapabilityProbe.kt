package me.shadow.cobalt.core.probe

import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLContext
import android.opengl.EGLDisplay
import android.opengl.EGLSurface
import android.opengl.GLES20
import android.opengl.GLES30
import android.os.Build
import android.os.Environment
import android.os.StatFs
import android.util.Log
import me.shadow.cobalt.core.model.*

/**
 * Default capability probe implementation.
 * Queries EGL/GLES versions, extensions, limits, surface support,
 * ABI, library availability, memory pressure, refresh rate, and driver info.
 *
 * GPU names may inform diagnostics but must not replace capability queries.
 */
class DefaultCapabilityProbe : CapabilityProbe {

    override fun collect(
        glesLibraryPath: String?,
        eglLibraryPath: String?
    ): CapabilitySnapshot {
        Log.i(TAG, "Collecting capability snapshot")

        val abi = Build.SUPPORTED_ABIS.firstOrNull() ?: "unknown"
        val apiLevel = Build.VERSION.SDK_INT

        // Probe EGL
        val eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)
        val eglVersion = if (eglDisplay != EGL14.EGL_NO_DISPLAY) {
            val versions = IntArray(2)
            EGL14.eglInitialize(eglDisplay, versions, 0, versions, 1)
            "${versions[0]}.${versions[1]}"
        } else "0.0"

        val eglVendor = EGL14.eglQueryString(eglDisplay, EGL14.EGL_VENDOR) ?: "unknown"
        val glesVersionString = EGL14.eglQueryString(eglDisplay, EGL14.EGL_VERSION) ?: "0.0"

        // Parse GLES version from EGL version string (format: "OpenGL ES X.Y ...")
        val glesParts = glesVersionString
            .substringAfter("OpenGL ES ", "")
            .substringBefore(" ")
            .split(".")
        val glesMajor = glesParts.getOrNull(0)?.toIntOrNull() ?: 0
        val glesMinor = glesParts.getOrNull(1)?.toIntOrNull() ?: 0

        // Collect extensions
        val extensions = mutableListOf<String>()
        if (glesMajor >= 3) {
            val extCount = IntArray(1)
            GLES30.glGetIntegerv(GLES30.GL_NUM_EXTENSIONS, extCount, 0)
            for (i in 0 until minOf(extCount[0], MAX_EXTENSIONS)) {
                val ext = GLES30.glGetStringi(GLES30.GL_EXTENSIONS, i)
                if (ext != null) extensions.add(ext)
            }
        } else if (glesMajor >= 2) {
            val extString = GLES20.glGetString(GLES20.GL_EXTENSIONS) ?: ""
            extensions.addAll(extString.split(" ").filter { it.isNotBlank() })
        }

        // Collect limits
        val maxTextureSize = IntArray(1).also { GLES20.glGetIntegerv(GLES20.GL_MAX_TEXTURE_SIZE, it, 0) }[0]
        val maxRenderBuffer = IntArray(1).also { GLES20.glGetIntegerv(GLES20.GL_MAX_RENDERBUFFER_SIZE, it, 0) }[0]
        val maxVertexAttrs = IntArray(1).also { GLES20.glGetIntegerv(GLES20.GL_MAX_VERTEX_ATTRIBS, it, 0) }[0]
        val maxTextureUnits = IntArray(1).also { GLES20.glGetIntegerv(GLES20.GL_MAX_TEXTURE_IMAGE_UNITS, it, 0) }[0]

        // Query GPU info
        val gpuVendor = GLES20.glGetString(GLES20.GL_VENDOR) ?: "unknown"
        val gpuRenderer = GLES20.glGetString(GLES20.GL_RENDERER) ?: "unknown"
        val driverVersion = GLES20.glGetString(GLES20.GL_VERSION) ?: "unknown"

        // Memory info
        val runtime = Runtime.getRuntime()
        val maxMemoryMb = runtime.maxMemory() / (1024 * 1024)
        val freeMemoryMb = runtime.freeMemory() / (1024 * 1024)
        val totalMemoryMb = runtime.totalMemory() / (1024 * 1024)
        val usedMemoryMb = totalMemoryMb - freeMemoryMb
        val availableMemoryMb = maxMemoryMb - usedMemoryMb

        // Storage
        val storageAvailable = try {
            val stat = StatFs(Environment.getDataDirectory().absolutePath)
            stat.availableBlocksLong * stat.blockSizeLong / (1024 * 1024)
        } catch (e: Exception) {
            0L
        }

        // Library availability
        val librariesLoaded = mapOf(
            "GLESv2" to isLibraryLoadable("libGLESv2.so"),
            "GLESv3" to isLibraryLoadable("libGLESv3.so"),
            "EGL" to isLibraryLoadable("libEGL.so"),
            "android" to isLibraryLoadable("libandroid.so"),
            "log" to isLibraryLoadable("liblog.so")
        )

        // Surface formats (simplified — real probe would query EGL configs)
        val surfaceFormats = listOf(0x3040) // GL_RGBA8 default

        // Cleanup EGL probe context
        if (eglDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglTerminate(eglDisplay)
        }

        val snapshot = CapabilitySnapshot(
            abi = abi,
            androidApiLevel = apiLevel,
            eglVendor = eglVendor,
            eglVersion = eglVersion,
            glesVersion = glesVersionString,
            glesMajor = glesMajor,
            glesMinor = glesMinor,
            glesExtensions = extensions,
            maxTextureSize = maxTextureSize,
            maxRenderBufferSize = maxRenderBuffer,
            maxVertexAttributes = maxVertexAttrs,
            maxTextureUnits = maxTextureUnits,
            surfaceFormats = surfaceFormats,
            supportsBufferAge = extensions.any { it.contains("buffer_age", ignoreCase = true) },
            supportsContextLoss = true, // Android supports this
            driverFingerprint = "$gpuVendor-$driverVersion",
            deviceModel = "${Build.MANUFACTURER} ${Build.MODEL}",
            gpuVendor = gpuVendor,
            gpuRenderer = gpuRenderer,
            memoryClassMb = (availableMemoryMb + usedMemoryMb).toInt(),
            availableMemoryMb = availableMemoryMb,
            refreshRateHz = 60f, // Default, real probe would query display refresh
            librariesLoaded = librariesLoaded
        )

        Log.i(TAG, "Capability snapshot: GLES $glesMajor.$glesMinor, ABI=$abi, " +
            "extensions=${extensions.size}, maxTex=$maxTextureSize, " +
            "availableMem=${availableMemoryMb}MB")

        return snapshot
    }

    override fun validateMinimums(snapshot: CapabilitySnapshot, family: RouteFamily): Boolean {
        return when (family) {
            RouteFamily.LEGACY -> {
                snapshot.glesMajor >= 1 &&
                    snapshot.librariesLoaded["GLESv2"] == true &&
                    snapshot.librariesLoaded["EGL"] == true
            }
            RouteFamily.MODERN -> {
                snapshot.glesMajor >= 3 &&
                    snapshot.librariesLoaded["EGL"] == true &&
                    snapshot.availableMemoryMb >= MIN_MODERN_MEMORY_MB
            }
        }
    }

    private fun isLibraryLoadable(name: String): Boolean {
        return try {
            System.loadLibrary(name.removePrefix("lib").removeSuffix(".so"))
            true
        } catch (e: UnsatisfiedLinkError) {
            false
        }
    }

    companion object {
        private const val TAG = "CobaltProbe"
        private const val MAX_EXTENSIONS = 512
        private const val MIN_MODERN_MEMORY_MB = 768L
    }
}
