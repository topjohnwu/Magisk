package com.topjohnwu.magisk.model.events.dialog

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration.EnvFix
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Magisk
import com.topjohnwu.magisk.view.MagiskDialog

class EnvFixDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) = dialog
        .applyTitle(R.string.env_fix_title)
        .applyMessage(R.string.env_fix_msg)
        .applyButton(MagiskDialog.ButtonType.POSITIVE) {
            titleRes = R.string.yes
            preventDismiss = true
            onClick {
                dialog.applyTitle(R.string.setup_title)
                    .applyMessage(R.string.setup_msg)
                    .resetButtons()
                    .cancellable(false)
                val lbm = LocalBroadcastManager.getInstance(dialog.context)
                lbm.registerReceiver(object : BroadcastReceiver() {
                    override fun onReceive(context: Context, intent: Intent?) {
                        dialog.dismiss()
                        lbm.unregisterReceiver(this)
                    }
                }, IntentFilter(DISMISS))
                DownloadService(dialog.context) { subject = Magisk(EnvFix) }
            }
        }
        .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
            titleRes = android.R.string.no
        }
        .let { Unit }

    companion object {
        const val DISMISS = "com.topjohnwu.magisk.ENV_DONE"
    }
}
