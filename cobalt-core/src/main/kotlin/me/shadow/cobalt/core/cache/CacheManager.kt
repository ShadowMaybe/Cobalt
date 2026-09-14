package me.shadow.cobalt.core.cache

import me.shadow.cobalt.core.model.RouteFamily

/**
 * Allocates isolated, versioned, atomic cache directories and removes corrupt entries.
 *
 * Cache namespace hierarchy:
 *   <cobalt-cache-root>/policy-v1/<route-family>/<provider-build-id>/
 *     <abi>/<egl-vendor>-<egl-version>/<gles-version>-<driver-fingerprint>/
 *       <shader-profile-hash>/metadata.json, shader-cache/, program-cache/
 *
 * A cache entry is reusable only when all namespace components match.
 * Cache files must not be shared between route families or incompatible builds.
 */
interface CacheManager {

    /**
     * Resolves the cache directory for a specific namespace.
     * Returns null if the directory cannot be created or validated.
     */
    fun resolveDirectory(
        cacheRoot: String,
        family: RouteFamily,
        providerBuildId: String,
        abi: String,
        eglVendor: String,
        eglVersion: String,
        glesVersion: String,
        driverFingerprint: String,
        shaderProfileHash: String
    ): CacheDirectory?

    /**
     * Validates that a cache entry is not corrupt and matches the current namespace.
     */
    fun validateEntry(
        directory: CacheDirectory,
        entryName: String
    ): CacheEntryStatus

    /**
     * Writes a cache entry atomically (temp-write, fsync, rename, checksum verify).
     */
    fun writeEntry(
        directory: CacheDirectory,
        entryName: String,
        data: ByteArray
    ): Boolean

    /**
     * Reads a cache entry, returning null if missing or corrupt.
     */
    fun readEntry(
        directory: CacheDirectory,
        entryName: String
    ): ByteArray?

    /**
     * Invalidates (deletes) a specific cache entry.
     */
    fun invalidateEntry(
        directory: CacheDirectory,
        entryName: String
    ): Boolean

    /**
     * Clears all cache entries for a namespace.
     */
    fun clearNamespace(directory: CacheDirectory): Boolean

    /**
     * Enforces size and age bounds on cache entries.
     * Evicts oldest/least-recently-used entries first.
     */
    fun enforceLimits(
        directory: CacheDirectory,
        maxSizeBytes: Long,
        maxAgeMillis: Long
    ): Long // returns bytes freed

    /**
     * Computes the cache namespace hash for diagnostics.
     */
    fun namespaceHash(
        family: RouteFamily,
        providerBuildId: String,
        abi: String,
        glesVersion: String,
        driverFingerprint: String,
        shaderProfileHash: String
    ): String
}

/**
 * Handle to a resolved cache directory.
 */
data class CacheDirectory(
    val path: String,
    val writable: Boolean,
    val valid: Boolean
)

enum class CacheEntryStatus {
    VALID,
    CORRUPT,
    MISSING,
    STALE,
    WRONG_NAMESPACE
}
