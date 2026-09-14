package me.shadow.cobalt.core.model

/**
 * Surface configuration for resize, resume, and surface recreation.
 */
data class SurfaceInfo(
    val width: Int,
    val height: Int,
    val format: Int = 0, // EGL surface format, 0 = default
    val isOpaque: Boolean = false,
    val surfaceHandle: Long = 0 // ANativeWindow or platform handle
)
