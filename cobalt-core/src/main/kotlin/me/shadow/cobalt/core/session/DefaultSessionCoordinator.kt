package me.shadow.cobalt.core.session

import me.shadow.cobalt.core.model.*
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock

/**
 * Default implementation of SessionCoordinator.
 * Serializes state transitions, owns session state, records transitions for diagnostics.
 */
class DefaultSessionCoordinator : SessionCoordinator {

    private data class SessionEntry(
        var state: SessionState = SessionState.NEW,
        val transitions: MutableList<StateTransition> = mutableListOf(),
        val version: VersionInfo,
        val family: RouteFamily,
        val probe: CapabilitySnapshot,
        val createdAt: Long = System.currentTimeMillis()
    )

    private val sessions = ConcurrentHashMap<String, SessionEntry>()
    private val lock = ReentrantLock()

    override fun allocate(
        input: LaunchInput,
        version: VersionInfo,
        family: RouteFamily,
        probe: CapabilitySnapshot
    ): SessionHandle {
        val handle = SessionHandle(
            gameVersion = version.normalizedKey ?: VersionKey(0, 0, 0),
            routeFamily = family
        )

        val entry = SessionEntry(
            state = SessionState.NEW,
            version = version,
            family = family,
            probe = probe
        )

        sessions[handle.id] = entry
        recordTransition(handle, SessionState.NEW, SessionState.NEW, "Session allocated")

        return handle
    }

    override fun transition(
        session: SessionHandle,
        to: SessionState,
        reason: String?
    ): Boolean {
        return lock.withLock {
            val entry = sessions[session.id] ?: return@withLock false
            val from = entry.state

            if (!isValidTransition(from, to)) {
                return@withLock false
            }

            entry.state = to
            entry.transitions.add(
                StateTransition(
                    from = from,
                    to = to,
                    timestampMillis = System.currentTimeMillis(),
                    reason = reason
                )
            )

            true
        }
    }

    override fun state(session: SessionHandle): SessionState {
        return sessions[session.id]?.state ?: SessionState.DESTROYED
    }

    override fun recordTransition(
        session: SessionHandle,
        from: SessionState,
        to: SessionState,
        reason: String?
    ) {
        lock.withLock {
            val entry = sessions[session.id] ?: return@withLock
            entry.transitions.add(
                StateTransition(
                    from = from,
                    to = to,
                    timestampMillis = System.currentTimeMillis(),
                    reason = reason
                )
            )
        }
    }

    override fun transitions(session: SessionHandle): List<StateTransition> {
        return sessions[session.id]?.transitions?.toList() ?: emptyList()
    }

    override fun release(session: SessionHandle) {
        lock.withLock {
            val entry = sessions[session.id] ?: return@withLock
            entry.state = SessionState.DESTROYED
            entry.transitions.add(
                StateTransition(
                    from = entry.state,
                    to = SessionState.DESTROYED,
                    timestampMillis = System.currentTimeMillis(),
                    reason = "Session released"
                )
            )
            // Keep entry for diagnostic purposes but could be pruned
        }
    }

    /**
     * Validates that a state transition is legal.
     */
    private fun isValidTransition(from: SessionState, to: SessionState): Boolean {
        return when (from) {
            SessionState.NEW -> to in setOf(SessionState.PROBING, SessionState.FAILED, SessionState.DESTROYING)
            SessionState.PROBING -> to in setOf(SessionState.SELECTED, SessionState.FAILED)
            SessionState.SELECTED -> to in setOf(SessionState.INITIALIZING, SessionState.FAILED)
            SessionState.INITIALIZING -> to in setOf(
                SessionState.READY,
                SessionState.DEGRADED,
                SessionState.FAILED,
                SessionState.DESTROYING
            )
            SessionState.READY -> to in setOf(
                SessionState.DEGRADED,
                SessionState.SUSPENDED,
                SessionState.DESTROYING
            )
            SessionState.DEGRADED -> to in setOf(
                SessionState.READY,
                SessionState.SUSPENDED,
                SessionState.DESTROYING
            )
            SessionState.SUSPENDED -> to in setOf(
                SessionState.READY,
                SessionState.DEGRADED,
                SessionState.FAILED,
                SessionState.DESTROYING
            )
            SessionState.FAILED -> to in setOf(SessionState.DESTROYING)
            SessionState.DESTROYING -> to in setOf(SessionState.DESTROYED)
            SessionState.DESTROYED -> false // terminal
        }
    }
}
