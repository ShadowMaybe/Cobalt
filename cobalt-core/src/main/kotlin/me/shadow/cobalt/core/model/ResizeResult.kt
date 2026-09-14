package me.shadow.cobalt.core.model

/**
 * Result of a surface resize operation.
 */
sealed class ResizeResult {
    data object OK : ResizeResult()
    data object SurfaceReconfigureFailed : ResizeResult()
    data class Error(val code: FailureCode, val message: String) : ResizeResult()
}
