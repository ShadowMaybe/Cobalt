package me.shadow.cobalt.core.model

/**
 * Neutral feature identifiers used in capability and degradation reporting.
 */
enum class FeatureId {
    SHADERS,
    MULTIDRAW,
    COMPUTE,
    TIMER_QUERIES,
    SURFACE_RECREATE,
    ALTERNATE_BACKEND,
    SHADER_CACHE,
    PROGRAM_CACHE,
    TEXTURE_ARRAY,
    INSTANCED_RENDERING,
    FRAMEBUFFER_FETCH,
    BUFFER_STORAGE
}
