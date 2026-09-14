package me.shadow.cobalt.core.config

import me.shadow.cobalt.core.model.Profile
import java.util.concurrent.ConcurrentHashMap

/**
 * Default configuration store backed by an in-memory map.
 * In production, this would persist to disk.
 *
 * Configuration is one-way: launcher/provider configs are converted into
 * Cobalt profiles at launch. Cobalt does not rewrite upstream JSON in place.
 */
class DefaultConfigurationStore : ConfigurationStore {

    private val store = ConcurrentHashMap<String, ConfigValue>()

    override fun get(key: String): ConfigValue? = store[key]

    override fun put(key: String, value: ConfigValue): Boolean {
        store[key] = value
        return true
    }

    override fun remove(key: String): Boolean {
        return store.remove(key) != null
    }

    override fun keys(): Set<String> = store.keys.toSet()

    override fun export(): Map<String, ConfigValue> = store.toMap()

    override fun import(config: Map<String, ConfigValue>): Int {
        var count = 0
        for ((key, value) in config) {
            if (isValidValue(value)) {
                store[key] = value
                count++
            }
        }
        return count
    }

    override fun profilePreset(profile: Profile): Map<String, ConfigValue> {
        return when (profile) {
            Profile.CONSERVATIVE -> mapOf(
                "enable_shaders" to ConfigValue.BooleanValue(false),
                "enable_multidraw" to ConfigValue.BooleanValue(false),
                "enable_compute" to ConfigValue.BooleanValue(false),
                "shader_cache_enabled" to ConfigValue.BooleanValue(false),
                "chunk_batch_size" to ConfigValue.IntValue(4),
                "max_texture_staging_mb" to ConfigValue.IntValue(32),
                "buffer_pool_size" to ConfigValue.IntValue(64)
            )
            Profile.STANDARD -> mapOf(
                "enable_shaders" to ConfigValue.BooleanValue(true),
                "enable_multidraw" to ConfigValue.BooleanValue(true),
                "enable_compute" to ConfigValue.BooleanValue(false),
                "shader_cache_enabled" to ConfigValue.BooleanValue(true),
                "chunk_batch_size" to ConfigValue.IntValue(16),
                "max_texture_staging_mb" to ConfigValue.IntValue(64),
                "buffer_pool_size" to ConfigValue.IntValue(128)
            )
            Profile.DEGRADED -> mapOf(
                "enable_shaders" to ConfigValue.BooleanValue(false),
                "enable_multidraw" to ConfigValue.BooleanValue(false),
                "enable_compute" to ConfigValue.BooleanValue(false),
                "shader_cache_enabled" to ConfigValue.BooleanValue(false),
                "chunk_batch_size" to ConfigValue.IntValue(4),
                "max_texture_staging_mb" to ConfigValue.IntValue(16),
                "buffer_pool_size" to ConfigValue.IntValue(32)
            )
            Profile.SAFE_MODE -> mapOf(
                "enable_shaders" to ConfigValue.BooleanValue(false),
                "enable_multidraw" to ConfigValue.BooleanValue(false),
                "enable_compute" to ConfigValue.BooleanValue(false),
                "shader_cache_enabled" to ConfigValue.BooleanValue(false),
                "chunk_batch_size" to ConfigValue.IntValue(2),
                "max_texture_staging_mb" to ConfigValue.IntValue(8),
                "buffer_pool_size" to ConfigValue.IntValue(16)
            )
        }
    }

    override fun migrate(fromVersion: Int, toVersion: Int, legacyConfig: Map<String, String>): Boolean {
        if (fromVersion >= toVersion) return true

        // Version-specific migrations
        for (version in fromVersion until toVersion) {
            when (version) {
                1 -> migrateV1ToV2(legacyConfig)
                2 -> migrateV2ToV3(legacyConfig)
                // Future versions added here
            }
        }
        return true
    }

    private fun migrateV1ToV2(config: Map<String, String>) {
        // Example migration: rename "use_shaders" -> "enable_shaders"
        config["use_shaders"]?.let { store["enable_shaders"] = ConfigValue.BooleanValue(it == "true") }
    }

    private fun migrateV2ToV3(config: Map<String, String>) {
        // Example migration: add default chunk_batch_size
        if (!store.containsKey("chunk_batch_size")) {
            store["chunk_batch_size"] = ConfigValue.IntValue(8)
        }
    }

    private fun isValidValue(value: ConfigValue): Boolean {
        return when (value) {
            is ConfigValue.StringValue -> value.value.isNotBlank()
            is ConfigValue.IntValue -> true
            is ConfigValue.LongValue -> true
            is ConfigValue.FloatValue -> true
            is ConfigValue.BooleanValue -> true
            is ConfigValue.MapValue -> value.value.all { isValidValue(it.value) }
        }
    }
}
