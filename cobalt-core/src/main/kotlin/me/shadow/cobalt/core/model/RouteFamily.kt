package me.shadow.cobalt.core.model

/**
 * Internal route family selected by the policy router.
 * Never exposed as a provider name — only LEGACY or MODERN.
 */
enum class RouteFamily {
    LEGACY,
    MODERN
}
