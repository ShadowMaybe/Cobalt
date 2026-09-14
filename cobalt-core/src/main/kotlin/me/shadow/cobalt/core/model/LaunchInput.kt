package me.shadow.cobalt.core.model

/**
 * Normalized launch input — translation of launcher-specific metadata
 * into a common format. All provider-specific values stay in ProviderConfig.
 */
data class LaunchInput(
    val gameVersionText: String,
    val launcherKind: LauncherKind,
    val surfaceProvider: SurfaceProvider,
    val javaProcessId: Int? = null,
    val requestedFeatures: Set<FeatureId> = emptySet(),
    val userSafeMode: Boolean = false,
    val suppliedLibraries: LibrarySet = LibrarySet(),
    val cacheRoot: String,
    val diagnosticLevel: DiagnosticLevel = DiagnosticLevel.INFO
)

/**
 * Handle to the platform surface provider (e.g., Android SurfaceView/SurfaceHolder).
 * Opaque to Cobalt internals except the surface/context manager.
 */
data class SurfaceProvider(
    val surfaceHandle: Long,
    val width: Int,
    val height: Int,
    val format: Int = 0
)

/**
 * Library set supplied by the launcher.
 */
data class LibrarySet(
    val nativeLibraries: Map<String, String> = emptyMap(), // name -> absolute path
    val glesLibraryPath: String? = null,
    val eglLibraryPath: String? = null,
    val providerBuildId: String? = null,
    val providerPackageDigest: String? = null
)
