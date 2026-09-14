package me.shadow.cobalt.core.api

import me.shadow.cobalt.core.model.RendererIdentity

/**
 * Factory for creating CobaltRenderer instances.
 * In plugin mode, this is invoked by the launch adapter.
 * In embedded mode, the launcher calls this directly.
 */
interface CobaltRendererFactory {

    /**
     * Creates a new CobaltRenderer instance.
     * The renderer internally initializes the version normalizer, policy router,
     * capability probe, and route adapters.
     */
    fun create(): CobaltRenderer

    /**
     * Returns the renderer identity without creating a full instance.
     * Useful for launcher metadata and discovery.
     */
    fun peekIdentity(): RendererIdentity
}
