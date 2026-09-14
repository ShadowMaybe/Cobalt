package me.shadow.cobalt.core.recovery

import android.util.Log
import me.shadow.cobalt.core.model.*

/**
 * Default recovery manager with bounded retries within a single route family.
 *
 * Budget: one fresh-context retry + one conservative-profile retry per launch.
 * A context failure, driver crash, or cache corruption must never trigger an
 * automatic switch to the other route family.
 */
class DefaultRecoveryManager : RecoveryManager {

    override fun attemptRecovery(
        session: SessionHandle,
        family: RouteFamily,
        error: FailureCode,
        currentRetryCount: Int
    ): RecoveryAction {
        Log.i(TAG, "Attempting recovery for session ${session.id}: " +
            "family=$family, error=$error, retry=$currentRetryCount")

        if (currentRetryCount >= maxRetries()) {
            Log.w(TAG, "Retry budget exhausted for session ${session.id}")
            return RecoveryAction.Fail(
                code = FailureCode.RETRY_BUDGET_EXHAUSTED,
                remediation = buildRemediation(error, family)
            )
        }

        return when (currentRetryCount) {
            0 -> {
                // First retry: fresh context
                Log.i(TAG, "Recovery F0: fresh context retry")
                RecoveryAction.FreshContextRetry
            }
            1 -> {
                // Second retry: conservative profile with cache disabled
                Log.i(TAG, "Recovery F1: conservative profile retry")
                RecoveryAction.ConservativeProfileRetry
            }
            2 -> {
                // Third retry: cache reset if cache-related error
                if (isCacheError(error)) {
                    Log.i(TAG, "Recovery F1b: cache reset retry")
                    RecoveryAction.CacheResetRetry(cacheNamespace = "default")
                } else {
                    Log.i(TAG, "Recovery F2: safe mode retry")
                    RecoveryAction.SafeModeRetry
                }
            }
            else -> {
                RecoveryAction.Fail(
                    code = error,
                    remediation = buildRemediation(error, family)
                )
            }
        }
    }

    override fun maxRetries(): Int = MAX_RETRIES

    override fun isExhausted(retryCount: Int): Boolean = retryCount >= MAX_RETRIES

    private fun isCacheError(error: FailureCode): Boolean {
        return error in setOf(
            FailureCode.CACHE_CORRUPTED,
            FailureCode.CACHE_WRITE_FAILED,
            FailureCode.CACHE_UNAVAILABLE
        )
    }

    private fun buildRemediation(error: FailureCode, family: RouteFamily): String {
        val familyName = family.name.lowercase()
        return when (error) {
            FailureCode.VERSION_UNRESOLVED ->
                "Select an explicit game version in launcher settings."
            FailureCode.VERSION_AMBIGUOUS ->
                "Launcher provided an ambiguous version. Use an exact version like '1.20.4'."
            FailureCode.UNSUPPORTED_ABI ->
                "Device ABI is not supported. Cobalt supports arm64-v8a, armeabi-v7a, x86_64, x86."
            FailureCode.EGL_CONTEXT_UNAVAILABLE ->
                "Cannot create EGL context. Check device GPU driver."
            FailureCode.GLES_TOO_LOW ->
                "Modern route requires GLES 3.0 minimum. This device may only support $familyName rendering."
            FailureCode.LIBRARY_MISSING ->
                "Required native library not found. Reinstall the Cobalt package."
            FailureCode.CONTEXT_CREATION_FAILED ->
                "EGL context creation failed. Try clearing the renderer cache."
            FailureCode.MINIMAL_DRAW_FAILED ->
                "Health check failed. The $familyName route may not be compatible with this device."
            FailureCode.NATIVE_CRASH ->
                "Native crash occurred. Collect a crash report and check compatibility."
            FailureCode.CACHE_CORRUPTED ->
                "Cache is corrupted. Clear the renderer cache and retry."
            FailureCode.MEMORY_PRESSURE ->
                "Insufficient memory. Close other apps and retry."
            else ->
                "An unexpected error occurred (${error.name}). Collect diagnostics and report."
        }
    }

    companion object {
        private const val TAG = "CobaltRecovery"
        private const val MAX_RETRIES = 2
    }
}
