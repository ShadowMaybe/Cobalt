package me.shadow.cobalt.core.model

/**
 * Source from which the game version was determined.
 */
enum class VersionSource {
    LAUNCHER_DESCRIPTOR,
    INSTALLATION_METADATA,
    USER_EXPLICIT,
    NONE
}
