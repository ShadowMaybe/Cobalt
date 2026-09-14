package me.shadow.cobalt.core.route

import me.shadow.cobalt.core.model.*

/**
 * Interface for a route adapter — maps neutral Cobalt calls to a selected
 * implementation family and its private configuration.
 *
 * The adapter must not become a public API or own Cobalt policy.
 * It owns translation and implementation-specific GL/EGL behavior.
 */
interface RouteAdapter {

    /**
     * Returns the route family this adapter implements.
     */
    fun family(): RouteFamily

    /**
     * Performs preflight checks before full initialization.
     * Validates library availability, entry points, and basic context support.
     */
    fun preflight(input: LaunchInput, probe: CapabilitySnapshot): PreflightDecision

    /**
     * Builds a private provider configuration for the selected profile.
     * This configuration never crosses the public API boundary.
     */
    fun buildPrivateConfig(
        input: LaunchInput,
        probe: CapabilitySnapshot,
        profile: Profile
    ): ProviderConfig

    /**
     * Initializes the adapter with the given configuration.
     * Loads native libraries, resolves function pointers, and installs hooks.
     */
    fun initialize(config: ProviderConfig): InitResult

    /**
     * Makes the adapter's EGL context current on the calling thread.
     */
    fun makeCurrent(): Result<Unit, FailureCode>

    /**
     * Translates a shader request from desktop-style to host-compatible form.
     */
    fun translateShader(request: ShaderRequest): ShaderResult

    /**
     * Submits a command buffer for rendering.
     */
    fun submit(commandBuffer: CommandBuffer): SubmitResult

    /**
     * Presents the current frame (swap buffers or equivalent).
     */
    fun present(presentInfo: PresentInfo): PresentResult

    /**
     * Suspends the adapter — releases thread-specific resources.
     */
    fun suspend()

    /**
     * Resumes the adapter with a new or restored surface.
     */
    fun resume(surfaceInfo: SurfaceInfo): ResumeResult

    /**
     * Destroys the adapter and releases all native resources.
     * Must be called during ordered teardown on the render thread.
     */
    fun destroy()

    /**
     * Collects diagnostic data from the adapter for crash reports.
     */
    fun collectDiagnostics(): ProviderDiagnostics
}

/**
 * Request for shader translation.
 */
data class ShaderRequest(
    val shaderId: String,
    val shaderType: ShaderType,
    val source: String,
    val profile: Profile
)

enum class ShaderType {
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESS_CONTROL,
    TESS_EVALUATION,
    COMPUTE
}

/**
 * Result of shader translation.
 */
sealed class ShaderResult {
    data class Supported(val translatedSource: String) : ShaderResult()
    data class Degraded(val translatedSource: String, val warnings: List<String>) : ShaderResult()
    data class Unsupported(val reason: String) : ShaderResult()
}

/**
 * Result of a command buffer submission.
 */
sealed class SubmitResult {
    data object OK : SubmitResult()
    data class Error(val code: FailureCode, val message: String) : SubmitResult()
}
