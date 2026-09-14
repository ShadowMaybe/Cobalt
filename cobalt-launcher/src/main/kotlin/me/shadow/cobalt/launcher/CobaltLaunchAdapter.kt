package me.shadow.cobalt.launcher

import me.shadow.cobalt.core.model.*

/**
 * Translates launcher-specific metadata into a normalized LaunchInput.
 * Does not make an independent route decision — that is owned by the policy router.
 *
 * Supports two deployment forms:
 * - Plugin/APK mode: discovers an Android package through application metadata
 * - Embedded mode: receives a library set, surface provider, version, and cache root directly
 */
interface CobaltLaunchAdapter {

    /**
     * Creates a normalized LaunchInput from plugin/APK metadata.
     * Validates the descriptor against an allowlist of expected library names,
     * supported ABI, package signature, and pinned provider build.
     *
     * @param metadata The launcher-specific metadata descriptor.
     * @param surfaceProvider Handle to the rendering surface.
     * @param cacheRoot Root directory for cache storage.
     * @return A normalized LaunchInput, or an error if the metadata is invalid.
     */
    fun fromPluginMetadata(
        metadata: PluginMetadata,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError>

    /**
     * Creates a normalized LaunchInput from an embedded launcher bridge.
     * The bridge supplies pinned libraries from an application-controlled directory.
     *
     * @param libraries The library set from the embedded bridge.
     * @param gameVersion The exact game version string.
     * @param surfaceProvider Handle to the rendering surface.
     * @param cacheRoot Root directory for cache storage.
     * @return A normalized LaunchInput.
     */
    fun fromEmbedded(
        libraries: LibrarySet,
        gameVersion: String,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<LaunchInput, LaunchAdapterError>

    /**
     * Validates that a plugin metadata descriptor matches expected format,
     * signature, and build identity.
     */
    fun validateDescriptor(metadata: PluginMetadata): DescriptorValidation

    /**
     * Returns the adapter kind (plugin or embedded).
     */
    fun kind(): LauncherKind
}

/**
 * Launcher-specific plugin metadata (from AndroidManifest or equivalent).
 */
data class PluginMetadata(
    val packageName: String,
    val versionName: String,
    val versionCode: Long,
    val rendererDescriptor: String,
    val nativeLibraryDir: String,
    val supportedAbis: List<String>,
    val signatureDigest: String?,
    val providerBuildId: String?,
    val environmentStrings: Map<String, String> = emptyMap()
)

/**
 * Descriptor validation result.
 */
data class DescriptorValidation(
    val valid: Boolean,
    val errors: List<String>,
    val warnings: List<String>
)

/**
 * Launch adapter errors.
 */
data class LaunchAdapterError(
    val code: AdapterErrorCode,
    val message: String,
    val remediation: String? = null
)

enum class AdapterErrorCode {
    INVALID_DESCRIPTOR,
    UNKNOWN_PACKAGE,
    SIGNATURE_MISMATCH,
    UNSUPPORTED_ABI,
    MISSING_NATIVE_LIBRARY,
    MISSING_GLES_LIBRARY,
    MISSING_EGL_LIBRARY,
    VERSION_CONFLICT,
    STORAGE_PERMISSION_DENIED,
    PACKAGE_REMOVED
}
