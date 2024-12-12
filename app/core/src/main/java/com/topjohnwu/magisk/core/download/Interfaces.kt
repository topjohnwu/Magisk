package com.topjohnwu.magisk.core.download

import android.app.Notification
import android.content.Context

interface DownloadSession {
    val context: Context
    fun attachNotification(id: Int, builder: Notification.Builder)
    fun onDownloadComplete()
}

interface DownloadNotifier {
    val context: Context
    fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit = {})
}
