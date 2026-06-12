package com.topjohnwu.magisk.core

import android.app.Notification
import android.content.Intent
import android.os.Build
import androidx.core.app.ServiceCompat
import androidx.core.content.IntentCompat
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.DownloadSession
import com.topjohnwu.magisk.core.download.Subject

class Service : BaseService(), DownloadSession {

    private var mEngine: DownloadEngine? = null
    override val context get() = this

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        if (intent.action == DownloadEngine.ACTION) {
            IntentCompat
                .getParcelableExtra(intent, DownloadEngine.SUBJECT_KEY, Subject::class.java)
                ?.let { subject ->
                    val engine = mEngine ?: DownloadEngine(this).also { mEngine = it }
                    engine.download(subject)
                }
        }
        return START_NOT_STICKY
    }

    override fun attachNotification(id: Int, builder: Notification.Builder) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
            builder.setForegroundServiceBehavior(Notification.FOREGROUND_SERVICE_IMMEDIATE)
        startForeground(id, builder.build())
    }

    override fun onDownloadComplete() {
        ServiceCompat.stopForeground(this, ServiceCompat.STOP_FOREGROUND_REMOVE)
    }
}
