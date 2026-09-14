package me.shadow.cobalt.core.router

import me.shadow.cobalt.core.model.*

/**
 * Default implementation of the fixed routing policy.
 * <= 1.17.0 → LEGACY, > 1.17.0 → MODERN.
 *
 * This router must never:
 * - Switch across route families during fallback
 * - Guess a version from a GPU, launcher label, or library presence
 * - Accept a successful library load as sufficient health evidence
 */
class DefaultPolicyRouter : PolicyRouter {

    override fun evaluate(
        version: VersionInfo,
        probe: CapabilitySnapshot,
        requestedFeatures: Set<FeatureId>
    ): RoutingDecision {
        // Version must be resolved
        if (version.isAmbiguous || version.normalizedKey == null) {
            return RoutingDecision(
                family = RouteFamily.LEGACY, // placeholder — will be rejected
                status = RoutingStatus.REJECT,
                profile = Profile.CONSERVATIVE,
                reason = FailureCode.VERSION_AMBIGUOUS,
                remediation = "Provide an exact game version. Ambiguous value: '${version.rawText}'"
            )
        }

        val family = policyFamily(version.normalizedKey)

        // Validate ABI
        if (probe.abi !in SUPPORTED_ABIS) {
            return RoutingDecision(
                family = family,
                status = RoutingStatus.REJECT,
                profile = Profile.CONSERVATIVE,
                reason = FailureCode.UNSUPPORTED_ABI,
                remediation = "Unsupported ABI: ${probe.abi}. Supported: ${SUPPORTED_ABIS.joinToString()}"
            )
        }

        // Validate EGL
        if (!probe.librariesLoaded["EGL"]!!) {
            return RoutingDecision(
                family = family,
                status = RoutingStatus.REJECT,
                profile = Profile.CONSERVATIVE,
                reason = FailureCode.EGL_CONTEXT_UNAVAILABLE,
                remediation = "EGL library not available."
            )
        }

        // Modern route requires GLES 3.0
        if (family == RouteFamily.MODERN && probe.glesMajor < 3) {
            return RoutingDecision(
                family = family,
                status = RoutingStatus.REJECT,
                profile = Profile.CONSERVATIVE,
                reason = FailureCode.GLES_TOO_LOW,
                remediation = "Modern route requires GLES 3.0 minimum. Device has GLES ${probe.glesMajor}.${probe.glesMinor}."
            )
        }

        // Determine profile based on capabilities and requested features
        val profile = determineProfile(family, probe, requestedFeatures)

        // Build capability plan
        val plan = buildCapabilityPlan(family, probe, requestedFeatures, profile)

        return RoutingDecision(
            family = family,
            status = if (profile == Profile.DEGRADED) RoutingStatus.DEGRADE else RoutingStatus.SELECT,
            profile = profile,
            plan = plan
        )
    }

    override fun policyFamily(versionKey: VersionKey): RouteFamily {
        return if (versionKey <= VersionKey.POLICY_BOUNDARY) {
            RouteFamily.LEGACY
        } else {
            RouteFamily.MODERN
        }
    }

    private fun determineProfile(
        family: RouteFamily,
        probe: CapabilitySnapshot,
        requestedFeatures: Set<FeatureId>
    ): Profile {
        val memoryOk = probe.availableMemoryMb >= if (family == RouteFamily.MODERN) 768L else 512L
        val glesOk = when (family) {
            RouteFamily.LEGACY -> probe.glesMajor >= 1
            RouteFamily.MODERN -> probe.glesMajor >= 3
        }

        if (!memoryOk || !glesOk) return Profile.CONSERVATIVE

        // Check if all requested features can be supported
        val allFeaturesSupported = requestedFeatures.all { feature ->
            when (feature) {
                FeatureId.SHADERS -> probe.glesMajor >= 2
                FeatureId.MULTIDRAW -> probe.glesMajor >= 3
                FeatureId.COMPUTE -> probe.glesMajor >= 3 && probe.glesMinor >= 1
                FeatureId.TIMER_QUERIES -> probe.glesExtensions.any { it.contains("timer_query", ignoreCase = true) }
                FeatureId.SURFACE_RECREATE -> true
                FeatureId.ALTERNATE_BACKEND -> false // requires explicit opt-in
                FeatureId.SHADER_CACHE -> probe.glesMajor >= 2
                FeatureId.PROGRAM_CACHE -> probe.glesMajor >= 2
                FeatureId.TEXTURE_ARRAY -> probe.glesMajor >= 3
                FeatureId.INSTANCED_RENDERING -> probe.glesMajor >= 3
                FeatureId.FRAMEBUFFER_FETCH -> probe.glesExtensions.any { it.contains("framebuffer_fetch", ignoreCase = true) }
                FeatureId.BUFFER_STORAGE -> probe.glesMajor >= 3 && probe.glesMinor >= 1
            }
        }

        return if (allFeaturesSupported) Profile.STANDARD else Profile.DEGRADED
    }

    private fun buildCapabilityPlan(
        family: RouteFamily,
        probe: CapabilitySnapshot,
        requestedFeatures: Set<FeatureId>,
        profile: Profile
    ): CapabilityPlan {
        val mandatory = mutableSetOf<FeatureId>()
        val enabled = mutableSetOf<FeatureId>()
        val degraded = mutableSetOf<FeatureId>()
        val blocked = mutableSetOf<FeatureId>()

        for (feature in FeatureId.entries) {
            val requested = feature in requestedFeatures
            val supported = isFeatureSupported(feature, family, probe)

            when {
                !supported && requested -> {
                    if (isMandatoryForWorkload(feature, family)) {
                        mandatory.add(feature) // will cause rejection elsewhere
                    } else {
                        degraded.add(feature)
                    }
                }
                supported && requested -> enabled.add(feature)
                !requested && supported -> { /* available but not requested */ }
                else -> blocked.add(feature)
            }
        }

        val glApiLabel = when (family) {
            RouteFamily.LEGACY -> "desktop-compat"
            RouteFamily.MODERN -> "gles3-compat"
        }

        return CapabilityPlan(
            mandatory = mandatory,
            enabled = enabled,
            degraded = degraded,
            blocked = blocked,
            glApiLabel = glApiLabel,
            profile = profile
        )
    }

    private fun isFeatureSupported(
        feature: FeatureId,
        family: RouteFamily,
        probe: CapabilitySnapshot
    ): Boolean = when (feature) {
        FeatureId.SHADERS -> probe.glesMajor >= 2
        FeatureId.MULTIDRAW -> probe.glesMajor >= 3
        FeatureId.COMPUTE -> probe.glesMajor >= 3 && probe.glesMinor >= 1
        FeatureId.TIMER_QUERIES -> probe.glesExtensions.any { it.contains("timer_query", ignoreCase = true) }
        FeatureId.SURFACE_RECREATE -> true
        FeatureId.ALTERNATE_BACKEND -> false // requires explicit external package
        FeatureId.SHADER_CACHE -> probe.glesMajor >= 2
        FeatureId.PROGRAM_CACHE -> probe.glesMajor >= 2
        FeatureId.TEXTURE_ARRAY -> probe.glesMajor >= 3
        FeatureId.INSTANCED_RENDERING -> probe.glesMajor >= 3
        FeatureId.FRAMEBUFFER_FETCH -> probe.glesExtensions.any { it.contains("framebuffer_fetch", ignoreCase = true) }
        FeatureId.BUFFER_STORAGE -> probe.glesMajor >= 3 && probe.glesMinor >= 1
    }

    private fun isMandatoryForWorkload(feature: FeatureId, family: RouteFamily): Boolean {
        // For now, no features are strictly mandatory — degraded is preferred over rejection
        return false
    }

    companion object {
        private val SUPPORTED_ABIS = setOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
    }
}
