package com.topjohnwu.magisk.core

import android.content.ContextWrapper
import android.content.Intent
import com.topjohnwu.magisk.core.base.BaseReceiver
import com.topjohnwu.magisk.core.download.Action
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.ktx.reboot
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.koin.core.inject

open class GeneralReceiver : BaseReceiver() {

    private val policyDB: PolicyDao by inject()

    private fun getPkg(intent: Intent): String {
        return intent.data?.encodedSchemeSpecificPart.orEmpty()
    }

    override fun onReceive(context: ContextWrapper, intent: Intent?) {
        intent ?: return

        fun rmPolicy(pkg: String) = GlobalScope.launch {
            policyDB.delete(pkg)
        }

        when (intent.action ?: return) {
            Intent.ACTION_REBOOT -> {
                SuCallbackHandler(context, intent.getStringExtra("action"), intent.extras)
            }
            Intent.ACTION_PACKAGE_REPLACED -> {
                // This will only work pre-O
                if (Config.suReAuth)
                    rmPolicy(getPkg(intent))
            }
            Intent.ACTION_PACKAGE_FULLY_REMOVED -> {
                val pkg = getPkg(intent)
                rmPolicy(pkg)
                Shell.su("magiskhide --rm $pkg").submit()
            }
            Intent.ACTION_LOCALE_CHANGED -> Shortcuts.setupDynamic(context)
            Const.Key.BROADCAST_MANAGER_UPDATE -> {
                intent.getParcelableExtra<ManagerJson>(Const.Key.INTENT_SET_APP)?.let {
                    Info.remote = Info.remote.copy(app = it)
                }
                DownloadService(context) {
                    subject = Subject.Manager(Action.APK.Upgrade)
                }
            }
            Const.Key.BROADCAST_REBOOT -> reboot()
        }
    }
}
