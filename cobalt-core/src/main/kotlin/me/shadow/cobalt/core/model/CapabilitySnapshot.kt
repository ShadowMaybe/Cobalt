package me.shadow.cobalt.core.model

/**
 * Raw capability data collected during probing.
 * Contains actual EGL/GLES query results — private diagnostic data.
 */
data class CapabilitySnapshot(
    val abi: String,
    val androidApiLevel: Int,
    val eglVendor: String,
    val eglVersion: String,
    val glesVersion: String,
    val glesMajor: Int,
    val glesMinor: Int,
    val glesExtensions: List<String>,
    val maxTextureSize: Int,
    val maxRenderBufferSize: Int,
    val maxVertexAttributes: Int,
    val maxTextureUnits: Int,
    val surfaceFormats: List<Int>,
    val supportsBufferAge: Boolean,
    val supportsContextLoss: Boolean,
    val driverFingerprint: String,
    val deviceModel: String,
    val gpuVendor: String,
    val gpuRenderer: String,
    val memoryClassMb: Int,
    val availableMemoryMb: Long,
    val refreshRateHz: Float,
    val librariesLoaded: Map<String, Boolean>,
    val collectedAtMillis: Long = System.currentTimeMillis()
)
