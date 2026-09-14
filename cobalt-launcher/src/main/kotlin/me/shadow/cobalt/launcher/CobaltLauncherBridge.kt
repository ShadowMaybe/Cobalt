package me.shadow.cobalt.launcher

import me.shadow.cobalt.core.api.CobaltRenderer
import me.shadow.cobalt.core.api.CobaltRendererFactory
import me.shadow.cobalt.core.model.*

/**
 * High-level entry point for launchers to create and manage Cobalt sessions.
 * Combines the adapter, factory, and session management into a single launcher-facing API.
 *
 * In plugin mode, the launcher discovers the Cobalt APK and invokes this bridge.
 * In embedded mode, the launcher links this bridge directly.
 */
class CobaltLauncherBridge(
    private val factory: CobaltRendererFactory,
    private val adapter: CobaltLaunchAdapter
) {
    private var renderer: CobaltRenderer? = null
    private var activeSession: SessionHandle? = null

    /**
     * Initializes the renderer. Must be called once before creating sessions.
     */
    fun initialize() {
        renderer = factory.create()
    }

    /**
     * Creates a session from plugin metadata.
     */
    fun launchFromPlugin(
        metadata: PluginMetadata,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<SessionHandle, LaunchError> {
        val launchInput = adapter.fromPluginMetadata(metadata, surfaceProvider, cacheRoot)
            .mapError { LaunchError(it.code.name, it.message, it.remediation) }
            .getOr { return Result.failure(it) }

        return createSession(launchInput)
    }

    /**
     * Creates a session from an embedded launcher bridge.
     */
    fun launchFromEmbedded(
        libraries: LibrarySet,
        gameVersion: String,
        surfaceProvider: SurfaceProvider,
        cacheRoot: String
    ): Result<SessionHandle, LaunchError> {
        val launchInput = adapter.fromEmbedded(libraries, gameVersion, surfaceProvider, cacheRoot)
            .mapError { LaunchError(it.code.name, it.message, it.remediation) }
            .getOr { return Result.failure(it) }

        return createSession(launchInput)
    }

    /**
     * Returns the active session, if any.
     */
    fun activeSession(): SessionHandle? = activeSession

    /**
     * Returns the renderer's public identity.
     */
    fun identity(): RendererIdentity = factory.peekIdentity()

    /**
     * Destroys the active session and releases resources.
     */
    fun destroySession() {
        activeSession?.let { renderer?.destroy(it) }
        activeSession = null
    }

    /**
     * Shuts down the renderer completely.
     */
    fun shutdown() {
        destroySession()
        renderer = null
    }

    private fun createSession(input: LaunchInput): Result<SessionHandle, LaunchError> {
        val r = renderer ?: return Result.failure(
            LaunchError("NOT_INITIALIZED", "Renderer not initialized. Call initialize() first.")
        )

        val session = r.createSession(input)
            .mapError { LaunchError(it.code.name, it.message, it.remediation) }
            .getOr { return Result.failure(it) }

        activeSession = session
        return Result.success(session)
    }
}

/**
 * Launcher-facing error type.
 */
data class LaunchError(
    val code: String,
    val message: String,
    val remediation: String? = null
)

// Helper extension for Result mapping
private inline fun <T, E, R> Result<T, E>.mapError(transform: (E) -> R): Result<T, R> {
    return fold(
        onSuccess = { Result.success(it) },
        onFailure = { Result.failure(transform(it)) }
    )
}

private inline fun <T, E> Result<T, E>.getOr(default: (E) -> Nothing): T {
    return fold(
        onSuccess = { it },
        onFailure = { default(it) }
    )
}
