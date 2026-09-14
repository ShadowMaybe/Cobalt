package me.shadow.cobalt.core.model

/**
 * Public renderer identity — always "Cobalt".
 * Provider names must never appear here.
 */
data class RendererIdentity(
    val name: String = "Cobalt",
    val policyVersion: String = "routing-policy-v1"
)
