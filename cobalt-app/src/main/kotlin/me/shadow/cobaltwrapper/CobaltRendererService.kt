package me.shadow.cobaltwrapper

import android.app.Service
import android.content.Intent
import android.os.Binder
import android.os.IBinder
import android.util.Log

/**
 * Android service that hosts the Cobalt renderer.
 * In plugin mode, the launcher binds to this service to interact with the renderer.
 *
 * The service does not own accounts, authentication, Java runtime, game installation,
 * controls, or UI — those remain in the host launcher.
 */
class CobaltRendererService : Service() {

    private val binder = LocalBinder()

    inner class LocalBinder : Binder() {
        fun getService(): CobaltRendererService = this@CobaltRendererService
    }

    override fun onBind(intent: Intent): IBinder {
        Log.i(TAG, "Launcher bound to Cobalt renderer service")
        return binder
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.i(TAG, "Cobalt renderer service started")
        return START_NOT_STICKY
    }

    override fun onDestroy() {
        Log.i(TAG, "Cobalt renderer service destroyed")
        super.onDestroy()
    }

    companion object {
        private const val TAG = "CobaltService"
    }
}
