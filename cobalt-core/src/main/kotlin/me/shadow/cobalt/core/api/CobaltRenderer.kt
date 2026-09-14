package me.shadow.cobalt.core.api

import me.shadow.cobalt.core.model.*

/**
 * Public Cobalt renderer facade.
 * Exposes stable renderer identity, neutral lifecycle calls, resource submission,
 * and status queries. Must never reveal provider names, environment variables,
 * or provider-specific JSON.
 *
 * All methods are thread-safe unless otherwise documented. GL-specific methods
 * (beginFrame, submit, endFrame) must be called from the owning render thread.
 */
interface CobaltRenderer {

    /**
     * Returns the stable renderer identity — always "Cobalt".
     */
    fun identity(): RendererIdentity

    /**
     * Creates a new renderer session from normalized launch input.
     * This triggers version normalization, route selection, capability probing,
     * adapter initialization, and health check before returning a usable handle.
     *
     * @param input Normalized launch parameters from the launcher adapter.
     * @return A session handle on success, or a structured error with remediation guidance.
     */
    fun createSession(input: LaunchInput): Result<SessionHandle, StartError>

    /**
     * Returns the neutral capability report for an active session.
     * Contains only public capability identifiers — no provider extension strings.
     */
    fun getCapabilities(session: SessionHandle): NeutralCapabilities

    /**
     * Begins a new frame. Must be called from the render thread.
     * Returns a frame token for tracking through submission and presentation.
     */
    fun beginFrame(session: SessionHandle, frameInfo: FrameInfo): Result<FrameToken, FrameError>

    /**
     * Submits a command buffer for the current frame. Must be called from the render thread.
     * The command buffer contents are provider-specific; Cobalt tracks ownership only.
     */
    fun submit(session: SessionHandle, commandBuffer: CommandBuffer): SubmitError?

    /**
     * Ends the current frame and presents. Must be called from the render thread.
     */
    fun endFrame(session: SessionHandle, presentInfo: PresentInfo): PresentResult

    /**
     * Resizes the rendering surface. May be called from the event/surface thread;
     * the implementation posts the resize to the render thread.
     */
    fun resize(session: SessionHandle, surfaceInfo: SurfaceInfo): ResizeResult

    /**
     * Suspends the session (e.g., app paused, surface lost).
     * The render thread must not issue GL calls after suspension.
     */
    fun suspend(session: SessionHandle, reason: SuspendReason): Unit

    /**
     * Resumes a suspended session with a new or restored surface.
     */
    fun resume(session: SessionHandle, surfaceInfo: SurfaceInfo): ResumeResult

    /**
     * Returns the current health snapshot for diagnostics and monitoring.
     */
    fun health(session: SessionHandle): HealthSnapshot

    /**
     * Destroys the session and releases all resources.
     * Performs ordered teardown: stop submissions, drain queue, flush telemetry,
     * close provider resources, destroy framebuffers/surfaces/context, close
     * libraries, finalize cache, mark DESTROYED.
     */
    fun destroy(session: SessionHandle): Unit
}
