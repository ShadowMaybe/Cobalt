package me.shadow.cobalt.core.cache

import android.util.Log
import me.shadow.cobalt.core.model.RouteFamily
import java.io.File
import java.security.MessageDigest

/**
 * Default cache manager implementation.
 * Allocates isolated, versioned, atomic cache directories and removes corrupt entries.
 *
 * Cache namespace: <root>/policy-v1/<family>/<buildId>/<abi>/<eglVendor>-<eglVersion>/
 *                  <glesVersion>-<driverFingerprint>/<shaderProfileHash>/
 *
 * All writes are atomic: temp-write, fsync, rename, checksum verify.
 */
class DefaultCacheManager : CacheManager {

    override fun resolveDirectory(
        cacheRoot: String,
        family: RouteFamily,
        providerBuildId: String,
        abi: String,
        eglVendor: String,
        eglVersion: String,
        glesVersion: String,
        driverFingerprint: String,
        shaderProfileHash: String
    ): CacheDirectory? {
        return try {
            val dir = File(
                cacheRoot,
                "policy-v1/${family.name.lowercase()}/$providerBuildId/" +
                    "$abi/${sanitize(eglVendor)}-$eglVersion/" +
                    "${glesVersion}-${sanitize(driverFingerprint)}/" +
                    shaderProfileHash
            )

            if (!dir.exists()) {
                val created = dir.mkdirs()
                if (!created) {
                    Log.e(TAG, "Failed to create cache directory: ${dir.absolutePath}")
                    return CacheDirectory(dir.absolutePath, writable = false, valid = false)
                }
            }

            // Validate write access
            val testFile = File(dir, ".cobalt_write_test")
            val canWrite = try {
                testFile.writeText("test")
                testFile.delete()
                true
            } catch (e: Exception) {
                false
            }

            CacheDirectory(
                path = dir.absolutePath,
                writable = canWrite,
                valid = true
            )
        } catch (e: Exception) {
            Log.e(TAG, "Failed to resolve cache directory", e)
            null
        }
    }

    override fun validateEntry(
        directory: CacheDirectory,
        entryName: String
    ): CacheEntryStatus {
        val file = File(directory.path, entryName)
        val checksumFile = File(directory.path, "$entryName.sha256")

        return when {
            !file.exists() -> CacheEntryStatus.MISSING
            !checksumFile.exists() -> CacheEntryStatus.CORRUPT
            else -> {
                try {
                    val data = file.readBytes()
                    val expectedChecksum = checksumFile.readText().trim()
                    val actualChecksum = sha256(data)

                    if (actualChecksum == expectedChecksum) {
                        CacheEntryStatus.VALID
                    } else {
                        Log.w(TAG, "Cache entry checksum mismatch: $entryName")
                        CacheEntryStatus.CORRUPT
                    }
                } catch (e: Exception) {
                    CacheEntryStatus.CORRUPT
                }
            }
        }
    }

    override fun writeEntry(
        directory: CacheDirectory,
        entryName: String,
        data: ByteArray
    ): Boolean {
        if (!directory.writable) return false

        val targetFile = File(directory.path, entryName)
        val tempFile = File(directory.path, "$entryName.tmp")
        val checksumFile = File(directory.path, "$entryName.sha256")

        return try {
            // Write to temp file
            tempFile.writeBytes(data)

            // Fsync (best effort on Android)
            try {
                tempFile.inputStream().channel.force(true)
            } catch (e: Exception) {
                // Fsync not supported on all Android filesystems
            }

            // Atomic rename
            val renamed = tempFile.renameTo(targetFile)
            if (!renamed) {
                tempFile.delete()
                return false
            }

            // Write checksum
            checksumFile.writeText(sha256(data))

            true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to write cache entry: $entryName", e)
            tempFile.delete()
            targetFile.delete()
            checksumFile.delete()
            false
        }
    }

    override fun readEntry(
        directory: CacheDirectory,
        entryName: String
    ): ByteArray? {
        val status = validateEntry(directory, entryName)
        return when (status) {
            CacheEntryStatus.VALID -> {
                try {
                    File(directory.path, entryName).readBytes()
                } catch (e: Exception) {
                    null
                }
            }
            CacheEntryStatus.CORRUPT -> {
                // Delete corrupt entry
                invalidateEntry(directory, entryName)
                null
            }
            else -> null
        }
    }

    override fun invalidateEntry(
        directory: CacheDirectory,
        entryName: String
    ): Boolean {
        val file = File(directory.path, entryName)
        val checksumFile = File(directory.path, "$entryName.sha256")
        file.delete()
        checksumFile.delete()
        return true
    }

    override fun clearNamespace(directory: CacheDirectory): Boolean {
        val dir = File(directory.path)
        return dir.deleteRecursively()
    }

    override fun enforceLimits(
        directory: CacheDirectory,
        maxSizeBytes: Long,
        maxAgeMillis: Long
    ): Long {
        val dir = File(directory.path)
        if (!dir.exists()) return 0

        var freedBytes = 0L
        val now = System.currentTimeMillis()

        // Collect all entries with metadata
        data class EntryInfo(val file: File, val checksum: File, val lastModified: Long, val size: Long)
        val entries = dir.listFiles()
            ?.filter { it.isFile && !it.name.startsWith(".") }
            ?.filter { !it.name.endsWith(".sha256") }
            ?.map { EntryInfo(it, File(dir, "${it.name}.sha256"), it.lastModified(), it.length()) }
            ?.sortedBy { it.lastModified } // oldest first
            ?: return 0

        // Calculate total size
        var totalSize = entries.sumOf { it.size }

        // Evict by age first
        for (entry in entries) {
            if (now - entry.lastModified > maxAgeMillis) {
                entry.file.delete()
                entry.checksum.delete()
                freedBytes += entry.size
                totalSize -= entry.size
            }
        }

        // Evict by size if still over limit
        if (totalSize > maxSizeBytes) {
            val remaining = entries.filter { it.file.exists() }.sortedBy { it.lastModified }
            for (entry in remaining) {
                if (totalSize <= maxSizeBytes) break
                entry.file.delete()
                entry.checksum.delete()
                freedBytes += entry.size
                totalSize -= entry.size
            }
        }

        return freedBytes
    }

    override fun namespaceHash(
        family: RouteFamily,
        providerBuildId: String,
        abi: String,
        glesVersion: String,
        driverFingerprint: String,
        shaderProfileHash: String
    ): String {
        val input = "$family/$providerBuildId/$abi/$glesVersion/$driverFingerprint/$shaderProfileHash"
        return sha256(input.toByteArray())
    }

    private fun sha256(data: ByteArray): String {
        val digest = MessageDigest.getInstance("SHA-256").digest(data)
        return digest.joinToString("") { "%02x".format(it) }
    }

    private fun sanitize(input: String): String {
        return input.replace(Regex("[^a-zA-Z0-9._-]"), "_")
    }

    companion object {
        private const val TAG = "CobaltCache"
    }
}
