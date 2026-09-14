package me.shadow.cobalt.core.config

import me.shadow.cobalt.core.model.Profile

/**
 * Stores and retrieves Cobalt configuration.
 * Configuration is one-way: launcher/provider configs are converted into
 * Cobalt profiles at launch. Cobalt does not rewrite upstream JSON in place.
 *
 * Configuration migration is explicit and tested. If a provider changes its
 * wire schema, the adapter version changes, the cache namespace changes,
 * and old settings are migrated through a tested codec or reset with notice.
 */
interface ConfigurationStore {

    /**
     * Reads a configuration value by key.
     */
    fun get(key: String): ConfigValue?

    /**
     * Writes a configuration value.
     */
    fun put(key: String, value: ConfigValue): Boolean

    /**
     * Removes a configuration value.
     */
    fun remove(key: String): Boolean

    /**
     * Returns all configuration keys.
     */
    fun keys(): Set<String>

    /**
     * Exports the current configuration as a map.
     */
    fun export(): Map<String, ConfigValue>

    /**
     * Imports configuration from a map, validating all values.
     * Returns the number of successfully imported entries.
     */
    fun import(config: Map<String, ConfigValue>): Int

    /**
     * Returns the profile preset for a given profile type.
     */
    fun profilePreset(profile: Profile): Map<String, ConfigValue>

    /**
     * Migrates configuration from a previous adapter version.
     * Returns true if migration was successful.
     */
    fun migrate(fromVersion: Int, toVersion: Int, legacyConfig: Map<String, String>): Boolean
}

/**
 * A configuration value — string, number, boolean, or nested map.
 */
sealed class ConfigValue {
    data class StringValue(val value: String) : ConfigValue()
    data class IntValue(val value: Int) : ConfigValue()
    data class LongValue(val value: Long) : ConfigValue()
    data class FloatValue(val value: Float) : ConfigValue()
    data class BooleanValue(val value: Boolean) : ConfigValue()
    data class MapValue(val value: Map<String, ConfigValue>) : ConfigValue()
}
