package me.shadow.cobalt.core.model

/**
 * Current health status of a renderer session.
 */
data class HealthSnapshot(
    val state: SessionState,
    val framesRendered: Long,
    val lastFrameTimeNanos: Long,
    val averageFrameTimeNanos: Long,
    val onePercentLowFrameTimeNanos: Long,
    val contextLossCount: Int,
    val shaderCompileWarnings: Int,
    val memoryPressureLevel: MemoryPressureLevel,
    val cacheHitRate: Float,
    val uptimeMillis: Long
)

enum class MemoryPressureLevel {
    NONE,
    LOW,
    MODERATE,
    HIGH,
    CRITICAL
}
