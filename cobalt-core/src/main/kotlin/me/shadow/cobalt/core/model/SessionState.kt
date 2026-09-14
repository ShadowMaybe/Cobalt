package me.shadow.cobalt.core.model

/**
 * Lifecycle states for a Cobalt renderer session.
 * Transitions are serialized by the session coordinator.
 */
enum class SessionState {
    NEW,
    PROBING,
    SELECTED,
    INITIALIZING,
    READY,
    DEGRADED,
    SUSPENDED,
    FAILED,
    DESTROYING,
    DESTROYED
}
