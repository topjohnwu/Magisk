package com.topjohnwu.magisk.model.receiver

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.data.database.base.su
import com.topjohnwu.magisk.data.repository.AppRepository
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.utils.DownloadApp
import com.topjohnwu.magisk.utils.SuLogger
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.utils.reboot
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell

open class GeneralReceiver : BroadcastReceiver() {

    private val appRepo: AppRepository by inject()

    companion object {
        const val REQUEST = "request"
        const val LOG = "log"
        const val NOTIFY = "notify"
        const val TEST = "test"
    }

    private fun getPkg(intent: Intent): String {
        return intent.data?.encodedSchemeSpecificPart ?: ""
    }

    override fun onReceive(context: Context, intent: Intent?) {
        if (intent == null)
            return
        var action: String? = intent.action ?: return
        when (action) {
            Intent.ACTION_REBOOT, Intent.ACTION_BOOT_COMPLETED -> {
                action = intent.getStringExtra("action")
                if (action == null) {
                    // Actual boot completed event
                    Shell.su("mm_patch_dtbo").submit { result ->
                        if (result.isSuccess)
                            Notifications.dtboPatched()
                    }
                    return
                }
                when (action) {
                    REQUEST -> {
                        val i = Intent(context, ClassMap[SuRequestActivity::class.java])
                                .setAction(action)
                                .putExtra("socket", intent.getStringExtra("socket"))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                .addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
                        context.startActivity(i)
                    }
                    LOG -> SuLogger.handleLogs(intent)
                    NOTIFY -> SuLogger.handleNotify(intent)
                    TEST -> Shell.su("magisk --use-broadcast").submit()
                }
            }
            Intent.ACTION_PACKAGE_REPLACED ->
                // This will only work pre-O
                if (Config.suReAuth)
                    appRepo.delete(getPkg(intent)).blockingGet()
            Intent.ACTION_PACKAGE_FULLY_REMOVED -> {
                val pkg = getPkg(intent)
                appRepo.delete(pkg).blockingGet()
                "magiskhide --rm $pkg".su().blockingGet()
            }
            Intent.ACTION_LOCALE_CHANGED -> Shortcuts.setup(context)
            Const.Key.BROADCAST_MANAGER_UPDATE -> {
                Info.remote = Info.remote.copy(app = Info.remote.app.copy(
                        link = intent.getStringExtra(Const.Key.INTENT_SET_LINK) ?: ""))
                DownloadApp.upgrade(intent.getStringExtra(Const.Key.INTENT_SET_NAME))
            }
            Const.Key.BROADCAST_REBOOT -> reboot()
        }
    }
}
