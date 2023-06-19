package com.topjohnwu.magisk.core.su

import android.content.Context
import android.os.Bundle
import android.widget.Toast
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.getLabel
import com.topjohnwu.magisk.core.ktx.getPackageInfo
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.model.su.createSuLog
import kotlinx.coroutines.runBlocking
import timber.log.Timber

object SuCallbackHandler {

    const val REQUEST = "request"
    const val LOG = "log"
    const val NOTIFY = "notify"

    fun run(context: Context, action: String?, data: Bundle?) {
        data ?: return

        // Debug messages
        if (BuildConfig.DEBUG) {
            Timber.d(action)
            data.let { bundle ->
                bundle.keySet().forEach {
                    Timber.d("[%s]=[%s]", it, bundle[it])
                }
            }
        }

        when (action) {
            LOG -> handleLogging(context, data)
            NOTIFY -> handleNotify(context, data)
        }
    }

    // https://android.googlesource.com/platform/frameworks/base/+/547bf5487d52b93c9fe183aa6d56459c170b17a4
    private fun Bundle.getIntComp(key: String, defaultValue: Int): Int {
        val value = get(key) ?: return defaultValue
        return when (value) {
            is Int -> value
            is Long -> value.toInt()
            else -> defaultValue
        }
    }

    private fun handleLogging(context: Context, data: Bundle) {
        val fromUid = data.getIntComp("from.uid", -1)
        val notify = data.getBoolean("notify", true)
        val policy = data.getIntComp("policy", SuPolicy.ALLOW)
        val toUid = data.getIntComp("to.uid", -1)
        val pid = data.getIntComp("pid", -1)
        val command = data.getString("command", "")
        val target = data.getIntComp("target", -1)
        val seContext = data.getString("context", "")
        val gids = data.getString("gids", "")

        val pm = context.packageManager

        val log = runCatching {
            pm.getPackageInfo(fromUid, pid)?.let {
                pm.createSuLog(it, toUid, pid, command, policy, target, seContext, gids)
            }
        }.getOrNull() ?: createSuLog(fromUid, toUid, pid, command, policy, target, seContext, gids)

        if (notify)
            notify(context, log.action == SuPolicy.ALLOW, log.appName)

        runBlocking { ServiceLocator.logRepo.insert(log) }
    }

    private fun handleNotify(context: Context, data: Bundle) {
        val uid = data.getIntComp("from.uid", -1)
        val pid = data.getIntComp("pid", -1)
        val policy = data.getIntComp("policy", SuPolicy.ALLOW)

        val pm = context.packageManager

        val appName = runCatching {
            pm.getPackageInfo(uid, pid)?.applicationInfo?.getLabel(pm)
        }.getOrNull() ?: "[UID] $uid"

        notify(context, policy == SuPolicy.ALLOW, appName)
    }

    private fun notify(context: Context, granted: Boolean, appName: String) {
        if (Config.suNotification == Config.Value.NOTIFICATION_TOAST) {
            val resId = if (granted)
                R.string.su_allow_toast
            else
                R.string.su_deny_toast

            context.toast(context.getString(resId, appName), Toast.LENGTH_SHORT)
        }
    }
}
