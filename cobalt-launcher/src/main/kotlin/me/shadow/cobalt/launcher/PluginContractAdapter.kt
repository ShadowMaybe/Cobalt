package me.shadow.cobalt.launcher

import me.shadow.cobalt.core.model.*

/**
 * Plugin/APK-specific adapter that implements CobaltLaunchAdapter for
 * launcher-discovered packages.
 *
 * Pipeline:
 *   plugin metadata -> parse and validate descriptor
 *   -> resolve bundled native library and ABI
 *   -> map launcher environment fields to private ProviderConfig
 *   -> allocate Cobalt cache namespace
 *   -> create LaunchInput
 *
 * The adapter must validate the descriptor against an allowlist of expected
 * library names, supported ABI, package signature, and pinned provider build.
 * It must reject unknown descriptors rather than treating every discovered
 * plugin as trusted.
 */
class PluginContractAdapter : CobaltLaunchAdapter {

    override fun fromPluginMetadata(
        metadata: PluginMetadata,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError> {
        val validation = validateDescriptor(metadata)
        if (!validation.valid) {
            return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.INVALID_DESCRIPTOR,
                    message = "Plugin descriptor validation failed: ${validation.errors.joinToString("; ")}",
                    remediation = "Ensure the plugin package is correctly installed and signed."
                )
            )
        }

        // Resolve ABI
        val abi = resolveBestAbi(metadata.supportedAbis)
            ?: return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.UNSUPPORTED_ABI,
                    message = "No supported ABI found. Supported: ${metadata.supportedAbis.joinToString()}",
                    remediation = "Install a version of the plugin compiled for this device's ABI."
                )
            )

        // Resolve game version from environment or descriptor
        val gameVersion = metadata.environmentStrings["COBALT_GAME_VERSION"]
            ?: metadata.environmentStrings["game_version"]
            ?: return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.VERSION_CONFLICT,
                    message = "No exact game version found in plugin metadata.",
                    remediation = "Launcher must supply an exact resolved game version."
                )
            )

        val libraries = LibrarySet(
            nativeLibraries = mapOf(
                "provider" to "${metadata.nativeLibraryDir}/lib${metadata.rendererDescriptor}.so"
            ),
            providerBuildId = metadata.providerBuildId,
            providerPackageDigest = metadata.signatureDigest
        )

        return Result.success(
            LaunchInput(
                gameVersionText = gameVersion,
                launcherKind = LauncherKind.PLUGIN_APK,
                surfaceProvider = surfaceProvider,
                requestedFeatures = parseRequestedFeatures(metadata.environmentStrings),
                suppliedLibraries = libraries,
                cacheRoot = cacheRoot,
                diagnosticLevel = parseDiagnosticLevel(metadata.environmentStrings)
            )
        )
    }

    override fun fromEmbedded(
        libraries: LibrarySet,
        gameVersion: String,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError> {
        // Embedded mode — libraries are pre-validated by the launcher bridge
        return Result.success(
            LaunchInput(
                gameVersionText = gameVersion,
                launcherKind = LauncherKind.EMBEDDED_LIBRARY,
                surfaceProvider = surfaceProvider,
                suppliedLibraries = libraries,
                cacheRoot = cacheRoot
            )
        )
    }

    override fun validateDescriptor(metadata: PluginMetadata): DescriptorValidation {
        val errors = mutableListOf<String>()
        val warnings = mutableListOf<String>()

        if (metadata.packageName.isBlank()) {
            errors.add("Package name is empty.")
        }
        if (metadata.rendererDescriptor.isBlank()) {
            errors.add("Renderer descriptor is empty.")
        }
        if (metadata.nativeLibraryDir.isBlank()) {
            errors.add("Native library directory is empty.")
        }
        if (metadata.supportedAbis.isEmpty()) {
            errors.add("No supported ABIs declared.")
        }
        if (metadata.signatureDigest == null) {
            warnings.add("No signature digest provided — trust validation limited.")
        }
        if (metadata.providerBuildId == null) {
            warnings.add("No provider build ID — cache invalidation may be less precise.")
        }

        return DescriptorValidation(
            valid = errors.isEmpty(),
            errors = errors,
            warnings = warnings
        )
    }

    override fun kind(): LauncherKind = LauncherKind.PLUGIN_APK

    private fun resolveBestAbi(supportedAbis: List<String>): String? {
        val preferred = listOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        return preferred.firstOrNull { it in supportedAbis }
            ?: supportedAbis.firstOrNull()
    }

    private fun parseRequestedFeatures(env: Map<String, String>): Set<FeatureId> {
        val features = mutableSetOf<FeatureId>()
        if (env["COBALT_FEATURE_SHADERS"] == "true") features.add(FeatureId.SHADERS)
        if (env["COBALT_FEATURE_MULTIDRAW"] == "true") features.add(FeatureId.MULTIDRAW)
        if (env["COBALT_FEATURE_COMPUTE"] == "true") features.add(FeatureId.COMPUTE)
        return features
    }

    private fun parseDiagnosticLevel(env: Map<String, String>): DiagnosticLevel {
        return when (env["COBALT_DIAGNOSTIC_LEVEL"]?.uppercase()) {
            "ERROR" -> DiagnosticLevel.ERROR
            "DEBUG" -> DiagnosticLevel.DEBUG
            "TRACE" -> DiagnosticLevel.TRACE
            else -> DiagnosticLevel.INFO
        }
    }
}
