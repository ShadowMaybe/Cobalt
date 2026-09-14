package me.shadow.cobalt.core.probe

import me.shadow.cobalt.core.model.CapabilitySnapshot
import me.shadow.cobalt.core.model.RouteFamily

/**
 * Discovers EGL/GLES versions, extensions, surface support, ABI,
 * library availability, memory pressure, refresh rate, and driver information.
 *
 * GPU names may inform diagnostics and profile lookup but must not
 * replace capability queries.
 *
 * The probe produces neutral capabilities — provider-specific symbols
 * and environment variables remain private.
 */
interface CapabilityProbe {

    /**
     * Collects a full capability snapshot from the current EGL/GLES context
     * and device information.
     *
     * @param glesLibraryPath Optional explicit path to GLES library for probing.
     * @param eglLibraryPath Optional explicit path to EGL library for probing.
     * @return A snapshot of all discoverable capabilities.
     */
    fun collect(
        glesLibraryPath: String? = null,
        eglLibraryPath: String? = null
    ): CapabilitySnapshot

    /**
     * Validates that a snapshot meets minimum requirements for a route family.
     * Returns true if the device can proceed, false if the family should be rejected.
     */
    fun validateMinimums(snapshot: CapabilitySnapshot, family: RouteFamily): Boolean
}
