package me.shadow.cobalt.core.performance

import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.os.PowerManager
import me.shadow.cobalt.core.model.SessionHandle
import java.util.concurrent.CopyOnWriteArrayList
import java.util.concurrent.atomic.AtomicLong

/**
 * Default performance controller.
 * Tracks frame timing, CPU/GPU balance, memory usage, and thermal state.
 * Enforces budget constraints and provides optimization recommendations.
 */
class DefaultPerformanceController : PerformanceController {

    private val frameTimes = CopyOnWriteArrayList<Long>()
    private val cpuTimes = CopyOnWriteArrayList<Long>()
    private val gpuTimes = CopyOnWriteArrayList<Long>()
    private val totalFrames = AtomicLong(0)
    private val droppedFrames = AtomicLong(0)
    private val startTime = AtomicLong(System.currentTimeMillis())
    private val peakMemory = AtomicLong(0)

    private var thermalState = ThermalState.NOMINAL
    private var lastFrameTimeNanos = 0L

    override fun recordFrame(
        frameIndex: Long,
        frameTimeNanos: Long,
        cpuTimeNanos: Long,
        gpuTimeNanos: Long?
    ) {
        frameTimes.add(frameTimeNanos)
        cpuTimes.add(cpuTimeNanos)
        gpuTimeNanos?.let { gpuTimes.add(it) }
        totalFrames.incrementAndGet()
        lastFrameTimeNanos = frameTimeNanos

        // Track dropped frames ( > 33ms = below 30fps)
        if (frameTimeNanos > DROPPED_FRAME_THRESHOLD_NS) {
            droppedFrames.incrementAndGet()
        }

        // Track peak memory
        val currentMemory = Runtime.getRuntime().let {
            it.totalMemory() - it.freeMemory()
        }
        peakMemory.updateAndGet { maxOf(it, currentMemory) }

        // Update thermal state periodically
        if (totalFrames.get() % 60 == 0L) {
            updateThermalState()
        }
    }

    override fun summary(): PerformanceSummary {
        val sortedFrameTimes = frameTimes.sorted()
        val sortedCpuTimes = cpuTimes.sorted()

        return PerformanceSummary(
            averageFrameTimeNanos = if (frameTimes.isNotEmpty()) frameTimes.average().toLong() else 0,
            medianFrameTimeNanos = sortedFrameTimes.getOrElse(sortedFrameTimes.size / 2) { 0 },
            onePercentLowFrameTimeNanos = sortedFrameTimes.getOrElse((sortedFrameTimes.size * 0.01).toInt()) { 0 },
            frameTimeVarianceNanos = calculateVariance(frameTimes),
            totalFramesRendered = totalFrames.get(),
            droppedFrames = droppedFrames.get().toInt(),
            averageCpuTimeNanos = if (cpuTimes.isNotEmpty()) cpuTimes.average().toLong() else 0,
            averageGpuTimeNanos = if (gpuTimes.isNotEmpty()) gpuTimes.average().toLong() else null,
            peakMemoryBytes = peakMemory.get(),
            currentMemoryBytes = Runtime.getRuntime().let { it.totalMemory() - it.freeMemory() },
            thermalState = thermalState,
            uptimeMillis = System.currentTimeMillis() - startTime.get()
        )
    }

    override fun isWithinBudget(): Boolean {
        val summary = summary()
        return summary.averageFrameTimeNanos <= TARGET_FRAME_TIME_NS &&
            summary.droppedFrames < MAX_DROPPED_FRAMES &&
            summary.thermalState != ThermalState.CRITICAL
    }

    override fun recommendations(): List<PerformanceRecommendation> {
        val recommendations = mutableListOf<PerformanceRecommendation>()
        val summary = summary()

        if (summary.averageFrameTimeNanos > TARGET_FRAME_TIME_NS) {
            recommendations.add(
                PerformanceRecommendation.ReduceShaderComplexity(
                    "Average frame time ${summary.averageFrameTimeNanos / 1_000_000}ms exceeds target ${TARGET_FRAME_TIME_NS / 1_000_000}ms"
                )
            )
        }

        if (summary.onePercentLowFrameTimeNanos > TARGET_FRAME_TIME_NS * 2) {
            recommendations.add(
                PerformanceRecommendation.ReduceChunkBatchSize(
                    currentSize = 16,
                    recommendedSize = 8
                )
            )
        }

        if (summary.droppedFrames > MAX_DROPPED_FRAMES) {
            recommendations.add(
                PerformanceRecommendation.DisableFeature(
                    feature = "shader-heavy features",
                    reason = "${summary.droppedFrames} frames dropped"
                )
            )
        }

        if (summary.thermalState == ThermalState.SERIOUS || summary.thermalState == ThermalState.CRITICAL) {
            recommendations.add(PerformanceRecommendation.EnableFramePacing)
        }

        if (summary.currentMemoryBytes > summary.peakMemoryBytes * 0.9) {
            recommendations.add(
                PerformanceRecommendation.IncreaseBufferPool(
                    "Memory usage at ${(summary.currentMemoryBytes * 100) / (Runtime.getRuntime().maxMemory())}% of max"
                )
            )
        }

        return recommendations
    }

    override fun reset() {
        frameTimes.clear()
        cpuTimes.clear()
        gpuTimes.clear()
        totalFrames.set(0)
        droppedFrames.set(0)
        startTime.set(System.currentTimeMillis())
        peakMemory.set(0)
        thermalState = ThermalState.NOMINAL
    }

    private fun calculateVariance(values: List<Long>): Long {
        if (values.size < 2) return 0
        val mean = values.average()
        return values.map { (it - mean).let { d -> d * d } }.average().toLong()
    }

    private fun updateThermalState() {
        // Simple heuristic based on frame time trends
        // In production, this would query PowerManager.THERMAL_STATUS_*
        val recentFrames = frameTimes.takeLast(60)
        if (recentFrames.size < 10) return

        val avgRecent = recentFrames.average()
        val avgOverall = frameTimes.average()

        thermalState = when {
            avgRecent > avgOverall * 2.0 -> ThermalState.CRITICAL
            avgRecent > avgOverall * 1.5 -> ThermalState.SERIOUS
            avgRecent > avgOverall * 1.2 -> ThermalState.FAIR
            else -> ThermalState.NOMINAL
        }
    }

    companion object {
        private const val TARGET_FRAME_TIME_NS = 16_666_666L // ~60fps
        private const val DROPPED_FRAME_THRESHOLD_NS = 33_333_333L // <30fps
        private const val MAX_DROPPED_FRAMES = 10
    }
}
