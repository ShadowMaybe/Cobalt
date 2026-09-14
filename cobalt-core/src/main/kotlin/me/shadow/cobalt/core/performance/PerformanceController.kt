package me.shadow.cobalt.core.performance

/**
 * Monitors and controls rendering performance.
 * Tracks frame time, CPU/GPU balance, memory, thermal behavior,
 * and enforces budget constraints.
 */
interface PerformanceController {

    /**
     * Records a frame sample for performance tracking.
     */
    fun recordFrame(
        frameIndex: Long,
        frameTimeNanos: Long,
        cpuTimeNanos: Long,
        gpuTimeNanos: Long? = null
    )

    /**
     * Returns the current performance summary.
     */
    fun summary(): PerformanceSummary

    /**
     * Checks if the renderer is within acceptable performance budgets.
     */
    fun isWithinBudget(): Boolean

    /**
     * Returns recommendations for reducing workload if performance is poor.
     */
    fun recommendations(): List<PerformanceRecommendation>

    /**
     * Resets all accumulated performance data.
     */
    fun reset()
}

data class PerformanceSummary(
    val averageFrameTimeNanos: Long,
    val medianFrameTimeNanos: Long,
    val onePercentLowFrameTimeNanos: Long,
    val frameTimeVarianceNanos: Long,
    val totalFramesRendered: Long,
    val droppedFrames: Int,
    val averageCpuTimeNanos: Long,
    val averageGpuTimeNanos: Long?,
    val peakMemoryBytes: Long,
    val currentMemoryBytes: Long,
    val thermalState: ThermalState,
    val uptimeMillis: Long
)

enum class ThermalState {
    NOMINAL,
    FAIR,
    SERIOUS,
    CRITICAL
}

sealed class PerformanceRecommendation {
    data class ReduceShaderComplexity(val reason: String) : PerformanceRecommendation()
    data class DisableFeature(val feature: String, val reason: String) : PerformanceRecommendation()
    data class ReduceChunkBatchSize(val currentSize: Int, val recommendedSize: Int) : PerformanceRecommendation()
    data class IncreaseBufferPool(val reason: String) : PerformanceRecommendation()
    data object EnableFramePacing : PerformanceRecommendation()
}
