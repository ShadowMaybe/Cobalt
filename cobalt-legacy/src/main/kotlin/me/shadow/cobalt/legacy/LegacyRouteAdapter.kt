package me.shadow.cobalt.legacy

import me.shadow.cobalt.core.model.*
import me.shadow.cobalt.core.route.RouteAdapter
import me.shadow.cobalt.core.route.ShaderRequest
import me.shadow.cobalt.core.route.ShaderResult
import me.shadow.cobalt.core.route.ShaderType
import me.shadow.cobalt.core.route.SubmitResult

/**
 * Legacy route adapter for Minecraft versions <= 1.17.0.
 *
 * Wraps the gl4es_extra_extra compatibility layer to provide desktop OpenGL
 * behavior over GLES 1.1 or GLES 2.0. Prioritizes older OpenGL behavior,
 * fixed-function compatibility, legacy shader conventions, stability, and
 * frame pacing.
 *
 * This adapter is compiled as a native compatibility shim. It requires
 * Cobalt to supply or locate the GLES/EGL libraries, select the intended
 * GLES level explicitly, and own Android surface/context setup.
 *
 * The adapter must not:
 * - Become a public API
 * - Own Cobalt policy decisions
 * - Switch to the modern route family under any circumstance
 */
class LegacyRouteAdapter : RouteAdapter {

    private var initialized = false
    private var diagnostics = ProviderDiagnostics(
        routeFamily = RouteFamily.LEGACY,
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

    override fun family(): RouteFamily = RouteFamily.LEGACY

    override fun preflight(input: LaunchInput, probe: CapabilitySnapshot): PreflightDecision {
        // Validate ABI
        if (probe.abi !in SUPPORTED_ABIS) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.UNSUPPORTED_ABI,
                remediation = "Legacy route requires one of: ${SUPPORTED_ABIS.joinToString()}"
            )
        }

        // Validate EGL context creation capability
        if (!probe.librariesLoaded["EGL"]!!) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.EGL_CONTEXT_UNAVAILABLE,
                remediation = "EGL library not loaded. Ensure the host provides a compatible EGL implementation."
            )
        }

        // Validate GLES library
        if (!probe.librariesLoaded["GLESv2"]!!) {
            return PreflightDecision(
                status = PreflightStatus.REJECT,
                reason = FailureCode.LIBRARY_MISSING,
                remediation = "GLESv2 library not found. Legacy route requires GLES 1.1 or 2.0."
            )
        }

        // Check memory
        if (probe.availableMemoryMb < MIN_MEMORY_MB) {
            return PreflightDecision(
                status = PreflightStatus.DEGRADE,
                reason = FailureCode.MEMORY_PRESSURE,
                remediation = "Low available memory. Some features may be disabled.",
                profile = Profile.CONSERVATIVE
            )
        }

        return PreflightDecision(
            status = PreflightStatus.ACCEPT,
            profile = Profile.CONSERVATIVE
        )
    }

    override fun buildPrivateConfig(
        input: LaunchInput,
        probe: CapabilitySnapshot,
        profile: Profile
    ): ProviderConfig {
        val glesLevel = when {
            probe.glesMajor >= 2 -> 2
            probe.glesMajor >= 1 -> 1
            else -> 0
        }

        val enableCache = profile != Profile.SAFE_MODE &&
            input.cacheRoot.isNotBlank() &&
            profile != Profile.CONSERVATIVE

        return ProviderConfig(
            routeFamily = RouteFamily.LEGACY,
            profile = profile,
            libraryPaths = input.suppliedLibraries.nativeLibraries +
                mapOf(
                    "GLESv2" to (input.suppliedLibraries.glesLibraryPath ?: ""),
                    "EGL" to (input.suppliedLibraries.eglLibraryPath ?: "")
                ),
            glesLevel = glesLevel,
            enableShaderCache = enableCache,
            enableProgramCache = enableCache,
            cacheDirectory = if (enableCache) "${input.cacheRoot}/legacy" else null,
            providerBuildId = input.suppliedLibraries.providerBuildId ?: "unknown"
        )
    }

    override fun initialize(config: ProviderConfig): InitResult {
        val startTime = System.currentTimeMillis()

        val glesPath = config.libraryPaths["GLESv2"] ?: ""
        val eglPath = config.libraryPaths["EGL"] ?: ""

        if (glesPath.isBlank() || eglPath.isBlank()) {
            return InitResult.Error(
                FailureCode.LIBRARY_MISSING,
                "GLES or EGL library path not provided"
            )
        }

        val success = nativeInitialize(glesPath, eglPath, config)
        if (!success) {
            return InitResult.Error(
                FailureCode.PROVIDER_INIT_FAILED,
                "Legacy native adapter initialization failed"
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
        val translated = nativeTranslateShader(request.shaderType.ordinal, request.source)

        return if (translated == request.source) {
            // Source unchanged — could mean no conversion needed or conversion not implemented
            diagnostics = diagnostics.copy(shaderCompileWarnings = diagnostics.shaderCompileWarnings + 1)
            ShaderResult.Degraded(translated, listOf("Shader passed through unchanged — conversion may not be implemented"))
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
        // Legacy adapter accepts all submissions — no filtering
        return SubmitResult.OK
    }

    override fun present(presentInfo: PresentInfo): PresentResult {
        // Legacy adapter delegates to surface context manager
        return PresentResult.OK
    }

    override fun suspend() {
        // Legacy adapter releases thread-specific resources
    }

    override fun resume(surfaceInfo: SurfaceInfo): ResumeResult {
        return ResumeResult.OK
    }

    override fun destroy() {
        if (initialized) {
            nativeDestroy()
            initialized = false
        }
    }

    override fun collectDiagnostics(): ProviderDiagnostics = diagnostics

    /**
     * Performs a minimal draw to verify the context is functional.
     */
    fun minimalDraw(): Boolean {
        return nativeMinimalDraw()
    }

    // JNI native methods — implemented in legacy_bridge.cpp
    private external fun nativeInitialize(
        glesLibraryPath: String,
        eglLibraryPath: String,
        config: ProviderConfig
    ): Boolean

    private external fun nativeMakeCurrent(): Boolean
    private external fun nativeTranslateShader(shaderType: Int, source: String): String
    private external fun nativeMinimalDraw(): Boolean
    private external fun nativeDestroy()
    private external fun nativeCollectDiagnostics(): String

    companion object {
        private val SUPPORTED_ABIS = setOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        private const val MIN_MEMORY_MB = 512L

        init {
            System.loadLibrary("cobalt-legacy")
        }
    }
}
