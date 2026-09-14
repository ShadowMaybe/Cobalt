package me.shadow.cobalt.core.model

/**
 * Result of route adapter initialization.
 */
sealed class InitResult {
    data object OK : InitResult()
    data class Error(val code: FailureCode, val message: String) : InitResult()
}
