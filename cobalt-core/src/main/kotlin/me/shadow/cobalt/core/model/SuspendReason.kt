package me.shadow.cobalt.core.model

/**
 * Reasons a session may be suspended.
 */
enum class SuspendReason {
    APP_PAUSED,
    SURFACE_LOST,
    CONTEXT_LOST,
    DRIVER_ERROR,
    RESOURCE_PRESSURE,
    USER_REQUEST
}
