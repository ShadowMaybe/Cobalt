package me.shadow.cobalt.modern

import me.shadow.cobalt.core.model.*
import me.shadow.cobalt.core.route.RouteAdapter
import me.shadow.cobalt.core.route.ShaderRequest
import me.shadow.cobalt.core.route.ShaderResult
import me.shadow.cobalt.core.route.SubmitResult

/**
 * Modern route adapter for Minecraft versions > 1.17.0.
 *
 * Wraps the MobileGlues GLES 3.x desktop-GL compatibility renderer.
 * Prioritizes raw performance, modern shader and framebuffer compatibility,
 * efficient chunk rendering, low overhead, and stable frame pacing.
 *
 * Requires host GLES 3.0 minimum; GLES 3.2 preferred.
 * Exposes optional diagnostics and feature switches through its native layer.
 *
 * Cobalt normalizes those switches into a versioned private profile rather
 * than exposing them directly.
 *
 * This adapter must not:
 * - Become a public API
 * - Own Cobalt policy decisions
 * - Switch to the legacy route family under any circumstance
 */
class ModernRouteAdapter : RouteAdapter {

    private var initialized = false
    private var config: ProviderConfig? = null
    private var diagnostics = ProviderDiagnostics(
        routeFamily = RouteFamily.MODERN,
        providerBuildId = "stub",
        initializationTimeMs = 0,
        shaderCompilationTimeMs = 0,
        cacheHits = 0,
        cacheMisses = 0,
        shaderCompileWarnings = 0,
        shaderCompileErrors = 0,
        contextLossCount = 0,
        extensionNegotiationLog = emptyList(),
        functionLoaderDecisions = emptyList()
    )

    override fun family(): RouteFamily = RouteFamily.MODERN

    override fun preflight(input: LaunchInput, probe: CapabilitySnapshot): PreflightDecision {
        // Validate ABI
        if (probe.abi !in SUPPORTED_ABIS) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.UNSUPPORTED_ABI,
                remediation = "Modern route requires one of: ${SUPPORTED_ABIS.joinToString()}"
            )
        }

        // Modern route requires GLES 3.0 minimum
        if (probe.glesMajor < 3) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.GLES_TOO_LOW,
                remediation = "Modern route requires GLES 3.0 minimum. Device reports GLES ${probe.glesMajor}.${probe.glesMinor}."
            )
        }

        // Validate EGL context creation
        if (!probe.librariesLoaded["EGL"]!!) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.EGL_CONTEXT_UNAVAILABLE,
                remediation = "EGL library not loaded."
            )
        }

        // Determine profile based on capabilities
        val profile = when {
            probe.glesMajor >= 3 && probe.glesMinor >= 2 -> Profile.STANDARD
            probe.glesMajor >= 3 -> Profile.CONSERVATIVE
            else -> Profile.DEGRADED
        }

        // Check memory for modern workload requirements
        if (probe.availableMemoryMb < MIN_MEMORY_MB) {
            return PreflightDecision(
                status = PreflightStatus.DEGRADE,
                reason = FailureCode.MEMORY_PRESSURE,
                remediation = "Low available memory for modern rendering pipeline.",
                profile = Profile.CONSERVATIVE
            )
        }

        return PreflightDecision(
            status = PreflightStatus.ACCEPT,
            profile = profile
        )
    }

    override fun buildPrivateConfig(
        input: LaunchInput,
        probe: CapabilitySnapshot,
        profile: Profile
    ): ProviderConfig {
        val enableCache = profile != Profile.SAFE_MODE &&
            input.cacheRoot.isNotBlank()

        val glesLevel = if (probe.glesMajor >= 3 && probe.glesMinor >= 2) 32 else 30

        return ProviderConfig(
            routeFamily = RouteFamily.MODERN,
            profile = profile,
            libraryPaths = input.suppliedLibraries.nativeLibraries +
                mapOf(
                    "GLESv3" to (input.suppliedLibraries.glesLibraryPath ?: ""),
                    "EGL" to (input.suppliedLibraries.eglLibraryPath ?: "")
                ),
            glesLevel = glesLevel,
            enableShaderCache = enableCache,
            enableProgramCache = enableCache,
            cacheDirectory = if (enableCache) "${input.cacheRoot}/modern" else null,
            providerBuildId = input.suppliedLibraries.providerBuildId ?: "unknown",
            customFlags = mapOf(
                "prefer_gles32" to (probe.glesMajor >= 3 && probe.glesMinor >= 2),
                "enable_multidraw" to (profile == Profile.STANDARD),
                "enable_compute" to (profile == Profile.STANDARD && probe.glesMajor >= 3),
                "chunk_batch_size" to if (profile == Profile.STANDARD) 16 else 8
            )
        )
    }

    override fun initialize(config: ProviderConfig): InitResult {
        val startTime = System.currentTimeMillis()
        this.config = config

        val success = nativeInitialize(config)
        if (!success) {
            return InitResult.Error(
                FailureCode.PROVIDER_INIT_FAILED,
                "Modern native adapter initialization failed"
            )
        }

        initialized = true
        diagnostics = diagnostics.copy(
            initializationTimeMs = System.currentTimeMillis() - startTime
        )

        return InitResult.OK
    }

    override fun makeCurrent(): Result<Unit, FailureCode> {
        if (!initialized) return Result.failure(FailureCode.PROVIDER_INIT_FAILED)
        return if (nativeMakeCurrent()) {
            Result.success(Unit)
        } else {
            Result.failure(FailureCode.CONTEXT_CREATION_FAILED)
        }
    }

    override fun translateShader(request: ShaderRequest): ShaderResult {
        val startTime = System.currentTimeMillis()
        val profileHash = config?.shaderProfileHash ?: "default"

        val translated = nativeTranslateShader(
            request.shaderType.ordinal,
            request.source,
            profileHash
        )

        return if (translated == request.source) {
            diagnostics = diagnostics.copy(shaderCompileWarnings = diagnostics.shaderCompileWarnings + 1)
            ShaderResult.Degraded(translated, listOf("Shader passed through unchanged"))
        } else {
            ShaderResult.Supported(translated)
        }.also {
            diagnostics = diagnostics.copy(
                shaderCompilationTimeMs = diagnostics.shaderCompilationTimeMs +
                    (System.currentTimeMillis() - startTime)
            )
        }
    }

    override fun submit(commandBuffer: CommandBuffer): SubmitResult {
        // Modern adapter accepts all submissions
        return SubmitResult.OK
    }

    override fun present(presentInfo: PresentInfo): PresentResult {
        return PresentResult.OK
    }

    override fun suspend() {
        // Release thread-specific resources
    }

    override fun resume(surfaceInfo: SurfaceInfo): ResumeResult {
        return ResumeResult.OK
    }

    override fun destroy() {
        if (initialized) {
            nativeDestroy()
            initialized = false
            config = null
        }
    }

    override fun collectDiagnostics(): ProviderDiagnostics = diagnostics

    /**
     * Performs a minimal draw to verify the GLES 3.x context is functional.
     */
    fun minimalDraw(): Boolean {
        return nativeMinimalDraw()
    }

    /**
     * Returns the required GLES major version.
     */
    fun requiredGlesMajor(): Int = nativeRequiredGlesMajor()

    /**
     * Returns the required GLES minor version.
     */
    fun requiredGlesMinor(): Int = nativeRequiredGlesMinor()

    // JNI native methods — implemented in modern_bridge.cpp
    private external fun nativeInitialize(config: ProviderConfig): Boolean
    private external fun nativeMakeCurrent(): Boolean
    private external fun nativeTranslateShader(shaderType: Int, source: String, profile: String): String
    private external fun nativeMinimalDraw(): Boolean
    private external fun nativeDestroy()
    private external fun nativeCollectDiagnostics(): String
    private external fun nativeRequiredGlesMajor(): Int
    private external fun nativeRequiredGlesMinor(): Int

    companion object {
        private val SUPPORTED_ABIS = setOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        private const val MIN_MEMORY_MB = 768L

        init {
            System.loadLibrary("cobalt-modern")
        }
    }
}
