package me.shadow.cobalt.launcher

import android.content.Context
import android.content.pm.PackageManager
import android.util.Log
import me.shadow.cobalt.core.model.*

/**
 * Real plugin contract adapter that handles the actual NGG-FCLRendererPlugin
 * and MobileGlues plugin manifest formats.
 *
 * Handles:
 * - fclPlugin=true manifest discovery flag
 * - renderer meta-data: "NAME:LIB1:LIB2" format
 * - boatEnv/pojavEnv colon-separated environment strings
 * - Native library extraction and validation
 * - ANGLE directory resolution
 *
 * Manifest contract (from NGG-FCLRendererPlugin):
 *   <meta-data android:name="fclPlugin"  android:value="true" />
 *   <meta-data android:name="des"        android:value="..." />
 *   <meta-data android:name="renderer"   android:value="NGGL4ES:libng_gl4es.so:libEGL.so" />
 *   <meta-data android:name="boatEnv"    android:value="KEY1=VAL1:KEY2=VAL2" />
 *   <meta-data android:name="pojavEnv"   android:value="KEY1=VAL1:KEY2=VAL2:..." />
 */
class RealPluginContractAdapter(
    private val context: Context? = null
) : CobaltLaunchAdapter {

    override fun fromPluginMetadata(
        metadata: PluginMetadata,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError> {
        // Validate descriptor
        val validation = validateDescriptor(metadata)
        if (!validation.valid) {
            return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.INVALID_DESCRIPTOR,
                    message = "Plugin validation failed: ${validation.errors.joinToString("; ")}",
                    remediation = "Ensure the plugin is correctly installed and signed."
                )
            )
        }

        // Parse renderer meta-data string: "NAME:LIB1:LIB2"
        val rendererInfo = parseRendererDescriptor(metadata.rendererDescriptor)
            ?: return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.INVALID_DESCRIPTOR,
                    message = "Cannot parse renderer descriptor: '${metadata.rendererDescriptor}'",
                    remediation = "Descriptor must be in format 'NAME:LIB1:LIB2'"
                )
            )

        // Resolve ABI
        val abi = resolveBestAbi(metadata.supportedAbis)
            ?: return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.UNSUPPORTED_ABI,
                    message = "No supported ABI. Device: ${android.os.Build.SUPPORTED_ABIS.joinToString()}, Plugin: ${metadata.supportedAbis.joinToString()}",
                    remediation = "Install a version compiled for this device's ABI."
                )
            )

        // Resolve native library paths
        val primaryLib = resolveLibraryPath(metadata.nativeLibraryDir, rendererInfo.primaryLibrary, abi)
        val secondaryLib = rendererInfo.secondaryLibrary?.let {
            resolveLibraryPath(metadata.nativeLibraryDir, it, abi)
        }

        // Parse environment strings (boatEnv or pojavEnv, preferring pojavEnv)
        val envStrings = parseEnvString(metadata.environmentStrings["pojavEnv"])
            .ifEmpty { parseEnvString(metadata.environmentStrings["boatEnv"]) }

        // Resolve game version from environment
        val gameVersion = metadata.environmentStrings["COBALT_GAME_VERSION"]
            ?: metadata.environmentStrings["game_version"]
            ?: envStrings["game_version"]
            ?: return Result.failure(
                LaunchAdapterError(
                    code = AdapterErrorCode.VERSION_CONFLICT,
                    message = "No exact game version in plugin metadata.",
                    remediation = "Launcher must supply an exact resolved game version."
                )
            )

        // Build library set
        val libraries = LibrarySet(
            nativeLibraries = mapOfNotNull(
                "provider" to primaryLib,
                rendererInfo.secondaryLibraryName to secondaryLib
            ),
            glesLibraryPath = secondaryLib,
            eglLibraryPath = secondaryLib,
            providerBuildId = metadata.providerBuildId,
            providerPackageDigest = metadata.signatureDigest
        )

        // Parse requested features from environment
        val requestedFeatures = parseRequestedFeatures(envStrings)

        // Parse diagnostic level
        val diagnosticLevel = parseDiagnosticLevel(envStrings)

        return Result.success(
            LaunchInput(
                gameVersionText = gameVersion,
                launcherKind = LauncherKind.PLUGIN_APK,
                surfaceProvider = surfaceProvider,
                requestedFeatures = requestedFeatures,
                suppliedLibraries = libraries,
                cacheRoot = cacheRoot,
                diagnosticLevel = diagnosticLevel
            )
        )
    }

    override fun fromEmbedded(
        libraries: LibrarySet,
        gameVersion: String,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError> {
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
        } else {
            val parts = metadata.rendererDescriptor.split(":")
            if (parts.size < 2) {
                errors.add("Renderer descriptor must have format 'NAME:LIB1[:LIB2]'.")
            } else {
                if (parts[0].isBlank()) errors.add("Renderer name is empty.")
                if (parts[1].isBlank()) errors.add("Primary library name is empty.")
                if (parts.size >= 3 && parts[2].isBlank()) errors.add("Secondary library name is empty.")
            }
        }
        if (metadata.nativeLibraryDir.isBlank()) {
            errors.add("Native library directory is empty.")
        }
        if (metadata.supportedAbis.isEmpty()) {
            errors.add("No supported ABIs declared.")
        }
        if (metadata.signatureDigest == null) {
            warnings.add("No signature digest — trust validation limited.")
        }
        if (metadata.providerBuildId == null) {
            warnings.add("No provider build ID — cache invalidation less precise.")
        }

        // Check for fclPlugin flag
        val hasFclPlugin = metadata.environmentStrings["fclPlugin"] == "true" ||
            metadata.environmentStrings["des"] != null
        if (!hasFclPlugin) {
            warnings.add("No fclPlugin flag detected — plugin may not be discoverable by FCL launchers.")
        }

        return DescriptorValidation(
            valid = errors.isEmpty(),
            errors = errors,
            warnings = warnings
        )
    }

    override fun kind(): LauncherKind = LauncherKind.PLUGIN_APK

    // ──────────────────────────────────────────────────────────────────────────
    // Private helpers
    // ──────────────────────────────────────────────────────────────────────────

    /**
     * Parses the renderer descriptor string "NAME:LIB1[:LIB2]".
     */
    private fun parseRendererDescriptor(descriptor: String): RendererInfo? {
        val parts = descriptor.split(":")
        if (parts.size < 2) return null

        val name = parts[0].trim()
        val primaryLib = parts[1].trim()
        val secondaryLib = if (parts.size >= 3) parts[2].trim() else null
        val secondaryLibName = secondaryLib?.let { "lib_${it.removePrefix("lib").removeSuffix(".so")}" }

        return RendererInfo(
            name = name,
            primaryLibrary = primaryLib,
            secondaryLibrary = secondaryLib,
            secondaryLibraryName = secondaryLibName
        )
    }

    /**
     * Parses colon-separated KEY=VALUE environment strings.
     * "LIBGL_ES=3:LIBGL_GL=31:LIBGL_NOERROR=1" -> map of key to value
     */
    private fun parseEnvString(envString: String?): Map<String, String> {
        if (envString.isNullOrBlank()) return emptyMap()

        return envString.split(":")
            .filter { it.contains("=") }
            .associate {
                val idx = it.indexOf('=')
                it.substring(0, idx).trim() to it.substring(idx + 1).trim()
            }
    }

    /**
     * Resolves the full path to a native library.
     */
    private fun resolveLibraryPath(nativeLibDir: String, libName: String, abi: String): String {
        val soName = if (libName.endsWith(".so")) libName else "lib$libName.so"
        return "$nativeLibDir/$soName"
    }

    /**
     * Selects the best ABI from supported list.
     */
    private fun resolveBestAbi(supportedAbis: List<String>): String? {
        val preferred = listOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        return preferred.firstOrNull { it in supportedAbis }
            ?: supportedAbis.firstOrNull()
    }

    /**
     * Parses requested features from environment strings.
     */
    private fun parseRequestedFeatures(env: Map<String, String>): Set<FeatureId> {
        val features = mutableSetOf<FeatureId>()

        // From MobileGlues config
        if (env["enableExtComputeShader"] == "1") features.add(FeatureId.COMPUTE)
        if (env["enableExtTimerQuery"] == "1") features.add(FeatureId.TIMER_QUERIES)
        if (env["enableExtDirectStateAccess"] == "1") features.add(FeatureId.MULTIDRAW)

        // From gl4es hints
        if (env["LIBGL_GLQUERIES"] == "1") features.add(FeatureId.TIMER_QUERIES)
        if (env["LIBGL_NOTEXARRAY"] != "1") features.add(FeatureId.TEXTURE_ARRAY)

        // From Cobalt-specific env
        if (env["COBALT_FEATURE_SHADERS"] == "true") features.add(FeatureId.SHADERS)
        if (env["COBALT_FEATURE_MULTIDRAW"] == "true") features.add(FeatureId.MULTIDRAW)

        return features
    }

    /**
     * Parses diagnostic level from environment strings.
     */
    private fun parseDiagnosticLevel(env: Map<String, String>): DiagnosticLevel {
        return when (env["COBALT_DIAGNOSTIC_LEVEL"]?.uppercase()) {
            "ERROR" -> DiagnosticLevel.ERROR
            "DEBUG" -> DiagnosticLevel.DEBUG
            "TRACE" -> DiagnosticLevel.TRACE
            else -> DiagnosticLevel.INFO
        }
    }

    private data class RendererInfo(
        val name: String,
        val primaryLibrary: String,
        val secondaryLibrary: String?,
        val secondaryLibraryName: String?
    )
}

private fun <K, V> mapOfNotNull(vararg pairs: Pair<K, V?>): Map<K, V> {
    return pairs.filterNotNull().filter { it.second != null }.associate { it.first to it.second!! }
}
