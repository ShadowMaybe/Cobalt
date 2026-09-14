package me.shadow.cobalt.core.session

import me.shadow.cobalt.core.model.*

/**
 * Serializes session startup, owns session state, coordinates teardown,
 * and publishes health.
 *
 * Must not issue GL calls from arbitrary application threads.
 * All state transitions are serialized and recorded.
 */
interface SessionCoordinator {

    /**
     * Allocates a new session and transitions it to NEW state.
     */
    fun allocate(
        input: LaunchInput,
        version: VersionInfo,
        family: RouteFamily,
        probe: CapabilitySnapshot
    ): SessionHandle

    /**
     * Transitions the session to a new state.
     * Returns false if the transition is invalid from the current state.
     */
    fun transition(
        session: SessionHandle,
        to: SessionState,
        reason: String? = null
    ): Boolean

    /**
     * Returns the current state of a session.
     */
    fun state(session: SessionHandle): SessionState

    /**
     * Records a state transition with timestamp for diagnostics.
     * Accepts a session ID string — may be called before the SessionHandle is allocated.
     */
    fun recordTransition(
        sessionId: String,
        from: SessionState,
        to: SessionState,
        reason: String? = null
    )

    /**
     * Returns all recorded state transitions for a session.
     */
    fun transitions(session: SessionHandle): List<StateTransition>

    /**
     * Marks the session as destroyed and releases coordinator resources.
     */
    fun release(session: SessionHandle)
}
