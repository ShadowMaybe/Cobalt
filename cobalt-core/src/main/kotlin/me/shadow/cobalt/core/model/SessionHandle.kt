package me.shadow.cobalt.core.model

import java.util.UUID

/**
 * Opaque handle to an active Cobalt renderer session.
 * Used by all public API methods to reference a session.
 */
data class SessionHandle(
    val id: String = UUID.randomUUID().toString(),
    val gameVersion: VersionKey,
    val routeFamily: RouteFamily,
    val createdAt: Long = System.currentTimeMillis()
)
