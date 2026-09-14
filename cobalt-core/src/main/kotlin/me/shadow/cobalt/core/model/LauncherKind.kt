package me.shadow.cobalt.core.model

/**
 * Identifies how the launcher discovered and integrated Cobalt.
 */
enum class LauncherKind {
    PLUGIN_APK,
    EMBEDDED_LIBRARY,
    UNKNOWN
}
