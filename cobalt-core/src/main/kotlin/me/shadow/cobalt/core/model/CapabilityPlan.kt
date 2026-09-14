package me.shadow.cobalt.core.model

/**
 * Intersection of host capabilities, provider capabilities, and policy constraints.
 * Determines which features are mandatory, enabled, degraded, or blocked.
 */
data class CapabilityPlan(
    val mandatory: Set<FeatureId>,
    val enabled: Set<FeatureId>,
    val degraded: Set<FeatureId>,
    val blocked: Set<FeatureId>,
    val glApiLabel: String,
    val profile: Profile
)
