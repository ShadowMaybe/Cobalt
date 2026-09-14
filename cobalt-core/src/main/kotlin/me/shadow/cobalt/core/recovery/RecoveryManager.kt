package me.shadow.cobalt.core.recovery

import me.shadow.cobalt.core.model.*

/**
 * Manages bounded retries, optional-feature reduction, shader-feature disablement,
 * cache invalidation, safe mode, and clean failure within a selected route family.
 *
 * Must never switch from legacy to modern or vice versa.
 * Retry budget: one fresh-context retry + one conservative-profile retry per launch.
 */
interface RecoveryManager {

    /**
     * Attempts a recovery within the selected route family.
     *
     * @param session The current session handle.
     * @param family The selected route family (must not change).
     * @param error The error that triggered recovery.
     * @param currentRetryCount How many retries have already been attempted.
     * @return A recovery action, or FAIL if the budget is exhausted.
     */
    fun attemptRecovery(
        session: SessionHandle,
        family: RouteFamily,
        error: FailureCode,
        currentRetryCount: Int
    ): RecoveryAction

    /**
     * Returns the maximum number of retries allowed per launch.
     */
    fun maxRetries(): Int

    /**
     * Checks if the session should be considered permanently failed.
     */
    fun isExhausted(retryCount: Int): Boolean
}

/**
 * Recovery actions that stay within the selected route family.
 */
sealed class RecoveryAction {
    /** Retry with the same configuration and a fresh context. */
    data object FreshContextRetry : RecoveryAction()

    /** Retry with cache disabled, optional extensions disabled, conservative profile. */
    data object ConservativeProfileRetry : RecoveryAction()

    /** Retry in safe mode with shader-heavy features blocked. */
    data object SafeModeRetry : RecoveryAction()

    /** Invalidate cache and retry. */
    data class CacheResetRetry(val cacheNamespace: String) : RecoveryAction()

    /** Recovery budget exhausted — stop the session. */
    data class Fail(val code: FailureCode, val remediation: String) : RecoveryAction()
}
