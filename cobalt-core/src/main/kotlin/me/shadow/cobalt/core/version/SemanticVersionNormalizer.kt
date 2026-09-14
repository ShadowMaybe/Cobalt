package me.shadow.cobalt.core.version

import me.shadow.cobalt.core.model.*

/**
 * Default implementation of VersionNormalizer.
 * Parses Minecraft version strings like "1.17", "1.17.1", "1.17.1-pre1", "1.20.4-rc1".
 *
 * Rejects non-numeric aliases such as "latest", "release", "snapshot" (without date),
 * or launcher display names unless they can be resolved to an exact version.
 */
class SemanticVersionNormalizer : VersionNormalizer {

    override fun normalize(rawVersion: String, source: VersionSource): VersionInfo {
        val trimmed = rawVersion.trim()

        if (trimmed.isBlank()) {
            return VersionInfo(
                rawText = rawVersion,
                normalizedKey = null,
                source = source,
                isAmbiguous = true
            )
        }

        // Reject non-numeric aliases
        if (ALIAS_REJECT.any { trimmed.equals(it, ignoreCase = true) }) {
            return VersionInfo(
                rawText = rawVersion,
                normalizedKey = null,
                source = source,
                isAmbiguous = true
            )
        }

        // Try to parse "X.Y.Z" or "X.Y" with optional suffix
        val versionRegex = Regex("""^(\d+)\.(\d+)(?:\.(\d+))?(?:[-.]?(pre|rc|alpha|beta|snapshot)(\d*))?$""", RegexOption.IGNORE_CASE)
        val match = versionRegex.matchEntire(trimmed)

        if (match != null) {
            val major = match.groupValues[1].toIntOrNull() ?: return ambiguousResult(rawVersion, source)
            val minor = match.groupValues[2].toIntOrNull() ?: return ambiguousResult(rawVersion, source)
            val patch = match.groupValues[3].toIntOrNull() ?: 0
            val suffix = match.groupValues[4].lowercase()

            val isSnapshot = suffix == "snapshot"
            val isPreRelease = suffix in listOf("pre", "rc", "alpha", "beta")

            val key = VersionKey(
                major = major,
                minor = minor,
                patch = patch,
                snapshot = isSnapshot,
                preRelease = isPreRelease
            )

            return VersionInfo(
                rawText = rawVersion,
                normalizedKey = key,
                source = source,
                isAmbiguous = false,
                parserVersion = parserVersion()
            )
        }

        // Could not parse
        return VersionInfo(
            rawText = rawVersion,
            normalizedKey = null,
            source = source,
            isAmbiguous = true
        )
    }

    override fun validateConsistency(
        primary: VersionInfo,
        secondary: VersionInfo
    ): VersionNormalizer.ValidationError? {
        if (primary.isAmbiguous || secondary.isAmbiguous) return null
        if (primary.normalizedKey == null || secondary.normalizedKey == null) return null

        return if (primary.normalizedKey != secondary.normalizedKey) {
            VersionNormalizer.ValidationError(
                message = "Version sources disagree: '${primary.rawText}' (${primary.normalizedKey}) vs '${secondary.rawText}' (${secondary.normalizedKey})",
                primaryRaw = primary.rawText,
                secondaryRaw = secondary.rawText
            )
        } else {
            null
        }
    }

    override fun parserVersion(): Int = 1

    private fun ambiguousResult(rawVersion: String, source: VersionSource): VersionInfo {
        return VersionInfo(
            rawText = rawVersion,
            normalizedKey = null,
            source = source,
            isAmbiguous = true,
            parserVersion = parserVersion()
        )
    }

    companion object {
        private val ALIAS_REJECT = listOf("latest", "release", "newest", "recommended")
    }
}
