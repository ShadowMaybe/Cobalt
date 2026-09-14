package me.shadow.cobalt.core.model

/**
 * Normalized Minecraft version represented as a comparable tuple.
 * Comparison uses strict numeric ordering: (1, 17, 1) > (1, 17, 0).
 */
data class VersionKey(
    val major: Int,
    val minor: Int,
    val patch: Int,
    val snapshot: Boolean = false,
    val preRelease: Boolean = false
) : Comparable<VersionKey> {

    override fun compareTo(other: VersionKey): Int {
        val majorCmp = major.compareTo(other.major)
        if (majorCmp != 0) return majorCmp
        val minorCmp = minor.compareTo(other.minor)
        if (minorCmp != 0) return minorCmp
        val patchCmp = patch.compareTo(other.patch)
        if (patchCmp != 0) return patchCmp
        // Snapshots and pre-releases sort after their base release
        return when {
            snapshot && !other.snapshot -> 1
            !snapshot && other.snapshot -> -1
            preRelease && !other.preRelease -> -1
            !preRelease && other.preRelease -> 1
            else -> 0
        }
    }

    companion object {
        val POLICY_BOUNDARY = VersionKey(1, 17, 0)
    }
}
