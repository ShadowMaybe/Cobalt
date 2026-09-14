package me.shadow.cobalt.core.version

import me.shadow.cobalt.core.model.RouteFamily
import me.shadow.cobalt.core.model.VersionKey

/**
 * Extension functions for VersionKey.
 */

/**
 * Returns the route family dictated by the routing policy for this version.
 */
fun VersionKey.routeFamily(): RouteFamily {
    return if (this <= VersionKey.POLICY_BOUNDARY) RouteFamily.LEGACY else RouteFamily.MODERN
}

/**
 * Returns true if this version is at or below the legacy/modern boundary.
 */
fun VersionKey.isLegacy(): Boolean = this <= VersionKey.POLICY_BOUNDARY

/**
 * Returns true if this version is above the legacy/modern boundary.
 */
fun VersionKey.isModern(): Boolean = this > VersionKey.POLICY_BOUNDARY

/**
 * Formats the version key as a human-readable string.
 */
fun VersionKey.toDisplayString(): String {
    val base = "$major.$minor.$patch"
    return when {
        snapshot -> "$base-snapshot"
        preRelease -> "$base-prerelease"
        else -> base
    }
}

/**
 * Parses a dot-separated version string into a VersionKey.
 * Returns null if parsing fails.
 */
fun String.parseVersionKey(): VersionKey? {
    val parts = trim().split(".")
    if (parts.size < 2 || parts.size > 3) return null

    val major = parts[0].toIntOrNull() ?: return null
    val minor = parts[1].toIntOrNull() ?: return null
    val patch = if (parts.size == 3) parts[2].toIntOrNull() ?: 0 else 0

    return VersionKey(major = major, minor = minor, patch = patch)
}
