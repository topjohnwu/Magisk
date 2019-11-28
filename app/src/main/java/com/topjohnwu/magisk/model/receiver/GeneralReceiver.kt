package com.topjohnwu.magisk.model.receiver

import android.content.ContextWrapper
import android.content.Intent
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.base.BaseReceiver
import com.topjohnwu.magisk.data.database.PolicyDao
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.utils.SuHandler
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import org.koin.core.inject

open class GeneralReceiver : BaseReceiver() {

    private val policyDB: PolicyDao by inject()

    private fun getPkg(intent: Intent): String {
        return intent.data?.encodedSchemeSpecificPart.orEmpty()
    }

    override fun onReceive(context: ContextWrapper, intent: Intent?) {
        intent ?: return

        when (intent.action ?: return) {
            Intent.ACTION_REBOOT -> {
                SuHandler(context, intent.getStringExtra("action"), intent.extras)
            }
            Intent.ACTION_PACKAGE_REPLACED -> {
                // This will only work pre-O
                if (Config.suReAuth)
                    policyDB.delete(getPkg(intent)).blockingGet()
            }
            Intent.ACTION_PACKAGE_FULLY_REMOVED -> {
                val pkg = getPkg(intent)
                policyDB.delete(pkg).blockingGet()
                Shell.su("magiskhide --rm $pkg").submit()
            }
            Intent.ACTION_LOCALE_CHANGED -> Shortcuts.setup(context)
            Const.Key.BROADCAST_MANAGER_UPDATE -> {
                intent.getParcelableExtra<ManagerJson>(Const.Key.INTENT_SET_APP)?.let {
                    Info.remote = Info.remote.copy(app = it)
                }
                DownloadService(context) {
                    subject = DownloadSubject.Manager(Configuration.APK.Upgrade)
                }
            }
            Const.Key.BROADCAST_REBOOT -> reboot()
        }
    }
}
