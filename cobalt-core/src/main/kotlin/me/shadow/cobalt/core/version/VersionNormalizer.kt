package me.shadow.cobalt.core.version

import me.shadow.cobalt.core.model.VersionInfo
import me.shadow.cobalt.core.model.VersionSource

/**
 * Parses and canonicalizes Minecraft game versions into comparable tuples.
 *
 * Detection order:
 * 1. Exact game-version value supplied by the launcher or game instance descriptor.
 * 2. Verified version parsed from the selected game installation metadata.
 * 3. Explicit user-selected version in a diagnostic or recovery flow.
 * 4. No route if sources disagree or are absent.
 *
 * The router must reject values such as "latest", "release", or a launcher
 * display alias unless the launcher also supplies an exact resolved version.
 */
interface VersionNormalizer {

    /**
     * Normalizes a raw version string into a VersionInfo.
     * Returns isAmbiguous=true if the version cannot be reliably resolved.
     *
     * @param rawVersion The raw version text from the launcher or installation.
     * @param source The source from which the version was obtained.
     * @return A VersionInfo with the normalized key or ambiguity flag.
     */
    fun normalize(rawVersion: String, source: VersionSource): VersionInfo

    /**
     * Validates that two version sources agree on the resolved version.
     * Returns null if they agree, or a descriptive error if they conflict.
     */
    fun validateConsistency(
        primary: VersionInfo,
        secondary: VersionInfo
    ): ValidationError?

    /**
     * The parser version supported by this normalizer.
     * Incremented when parsing behavior changes.
     */
    fun parserVersion(): Int

    data class ValidationError(
        val message: String,
        val primaryRaw: String,
        val secondaryRaw: String
    )
}
