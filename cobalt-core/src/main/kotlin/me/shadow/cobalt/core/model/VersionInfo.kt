package me.shadow.cobalt.core.model

/**
 * Result of version normalization — contains both the raw input and normalized key.
 */
data class VersionInfo(
    val rawText: String,
    val normalizedKey: VersionKey?,
    val source: VersionSource,
    val isAmbiguous: Boolean = false,
    val parserVersion: Int = 1
)
