package me.shadow.cobalt

import android.app.Activity
import android.os.Bundle
import android.util.Log

/**
 * Minimal test activity for Cobalt renderer.
 * This is NOT the final launcher — Cobalt is a library.
 */
class TestActivity : Activity() {

    private var renderer: CobaltRenderer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("Cobalt", "TestActivity onCreate")

        // Initialize with a test version
        renderer = CobaltRenderer.getInstance()
        val success = renderer?.init(1, 20, 1)
        Log.i("Cobalt", "Renderer init: $success")
        Log.i("Cobalt", "Version: ${renderer?.getVersion()}")
        Log.i("Cobalt", "Name: ${renderer?.getRendererName()}")
    }

    override fun onDestroy() {
        renderer?.shutdown()
        super.onDestroy()
        Log.i("Cobalt", "TestActivity onDestroy")
    }
}
