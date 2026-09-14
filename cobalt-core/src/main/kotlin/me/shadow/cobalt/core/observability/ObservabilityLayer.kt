package me.shadow.cobalt.core.observability

import me.shadow.cobalt.core.model.*

/**
 * Emits structured state transitions, capability reports, health probes,
 * and crash bundles. Must not log secrets, full external paths,
 * or unbounded shader source by default.
 *
 * Production default is DiagnosticLevel.INFO with sensitive data redacted.
 * A support bundle may include sanitized launch reports, bounded native log,
 * device/driver identifiers, route decision, and last successful frame marker.
 */
interface ObservabilityLayer {

    /**
     * Emits a structured event.
     */
    fun emit(event: CobaltEvent)

    /**
     * Records a state transition for the session.
     */
    fun recordTransition(
        sessionId: String,
        from: SessionState,
        to: SessionState,
        reason: String? = null
    )

    /**
     * Emits a capability snapshot report.
     */
    fun reportCapabilities(
        sessionId: String,
        probe: CapabilitySnapshot,
        plan: CapabilityPlan
    )

    /**
     * Emits a launch report with all structured fields.
     */
    fun emitLaunchReport(report: LaunchReport)

    /**
     * Collects a crash bundle for the given session.
     * Returns sanitized diagnostic data suitable for support transmission.
     */
    fun collectCrashBundle(sessionId: String): CrashBundle

    /**
     * Sets the diagnostic verbosity level for subsequent events.
     */
    fun setLevel(level: DiagnosticLevel)

    /**
     * Flushes any buffered diagnostic events.
     */
    fun flush()
}

/**
 * A structured Cobalt event for the diagnostic event stream.
 */
sealed class CobaltEvent {
    abstract val sessionId: String
    abstract val timestampMillis: Long

    data class SessionNew(
        override val sessionId: String,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class CapabilitySnapshot(
        override val sessionId: String,
        val snapshot: me.shadow.cobalt.core.model.CapabilitySnapshot,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class RouteSelected(
        override val sessionId: String,
        val family: RouteFamily,
        val profile: Profile,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class RouteReady(
        override val sessionId: String,
        val summary: String,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class RouteFailed(
        override val sessionId: String,
        val code: FailureCode,
        val remediation: String?,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class HealthCheck(
        override val sessionId: String,
        val passed: Boolean,
        val details: String?,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class StateTransition(
        override val sessionId: String,
        val from: SessionState,
        val to: SessionState,
        val reason: String?,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class CacheEvent(
        override val sessionId: String,
        val action: String,
        val detail: String?,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class ShaderEvent(
        override val sessionId: String,
        val shaderId: String,
        val result: String, // SUPPORTED, DEGRADED, UNSUPPORTED
        val warnings: List<String> = emptyList(),
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class PerformanceSample(
        override val sessionId: String,
        val frameIndex: Long,
        val frameTimeNanos: Long,
        val cpuTimeNanos: Long,
        val gpuTimeNanos: Long?,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()

    data class Error(
        override val sessionId: String,
        val code: FailureCode,
        val message: String,
        val recoverable: Boolean,
        override val timestampMillis: Long = System.currentTimeMillis()
    ) : CobaltEvent()
}

/**
 * Sanitized crash bundle for support transmission.
 */
data class CrashBundle(
    val sessionId: String,
    val launchReport: LaunchReport,
    val recentEvents: List<CobaltEvent>,
    val nativeLogTail: String,
    val deviceInfo: DeviceInfo,
    val collectedAtMillis: Long = System.currentTimeMillis()
)

data class DeviceInfo(
    val model: String,
    val manufacturer: String,
    val androidVersion: String,
    val apiLevel: Int,
    val gpuVendor: String,
    val gpuRenderer: String,
    val driverVersion: String,
    val abi: String,
    val totalMemoryMb: Long
)
