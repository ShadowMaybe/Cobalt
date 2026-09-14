package me.shadow.cobaltwrapper

import android.app.Application
import android.util.Log

/**
 * Application entry point for the standalone Cobalt APK.
 * When used as a plugin, the host launcher discovers this package
 * and invokes the renderer through the adapter contract.
 */
class CobaltApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        Log.i(TAG, "Cobalt renderer application started")
        Log.i(TAG, "Policy version: routing-policy-v1")
        Log.i(TAG, "Renderer identity: Cobalt")
    }

    companion object {
        private const val TAG = "Cobalt"
    }
}
