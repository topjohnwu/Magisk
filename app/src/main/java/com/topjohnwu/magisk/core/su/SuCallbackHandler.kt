package com.topjohnwu.magisk.core.su

import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.Process
import android.widget.Toast
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.ProviderCallHandler
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.toPolicy
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.startActivity
import com.topjohnwu.magisk.extensions.startActivityWithRoot
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.model.entity.toLog
import com.topjohnwu.superuser.Shell
import timber.log.Timber

object SuCallbackHandler : ProviderCallHandler {

    const val REQUEST = "request"
    const val LOG = "log"
    const val NOTIFY = "notify"
    const val TEST = "test"

    override fun call(context: Context, method: String, arg: String?, extras: Bundle?): Bundle? {
        invoke(context.wrap(), method, extras)
        return Bundle.EMPTY
    }

    operator fun invoke(context: Context, action: String?, data: Bundle?) {
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
            REQUEST -> {
                val intent = context.intent<SuRequestActivity>()
                    .setAction(action)
                    .putExtras(data)
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    .addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
                if (Build.VERSION.SDK_INT >= 29) {
                    // Android Q does not allow starting activity from background
                    intent.startActivityWithRoot()
                } else {
                    intent.startActivity(context)
                }
            }
            LOG -> handleLogs(context, data)
            NOTIFY -> handleNotify(context, data)
            TEST -> {
                val mode = data.getInt("mode", 2)
                Shell.su(
                    "magisk --connect-mode $mode",
                    "magisk --use-broadcast"
                ).submit()
            }
        }
    }

    private fun Any?.toInt(): Int? {
        return when (this) {
            is Int -> this
            is Long -> this.toInt()
            else -> null
        }
    }

    private fun handleLogs(context: Context, data: Bundle) {
        val fromUid = data["from.uid"].toInt() ?: return
        if (fromUid == Process.myUid())
            return

        val pm = context.packageManager

        val notify = data.getBoolean("notify", true)
        val allow = data["policy"].toInt() ?: return

        val policy = runCatching { fromUid.toPolicy(pm, allow) }.getOrElse { return }

        if (notify)
            notify(context, policy)

        val toUid = data["to.uid"].toInt() ?: return
        val pid = data["pid"].toInt() ?: return

        val command = data.getString("command") ?: return
        val log = policy.toLog(
            toUid = toUid,
            fromPid = pid,
            command = command
        )

        val logRepo = get<LogRepository>()
        logRepo.insert(log).subscribeK(onError = { Timber.e(it) })
    }

    private fun handleNotify(context: Context, data: Bundle) {
        val fromUid = data["from.uid"].toInt() ?: return
        if (fromUid == Process.myUid())
            return

        val pm = context.packageManager
        val allow = data["policy"].toInt() ?: return

        runCatching {
            val policy = fromUid.toPolicy(pm, allow)
            if (policy.policy >= 0)
                notify(context, policy)
        }
    }

    private fun notify(context: Context, policy: MagiskPolicy) {
        if (policy.notification && Config.suNotification == Config.Value.NOTIFICATION_TOAST) {
            val resId = if (policy.policy == MagiskPolicy.ALLOW)
                R.string.su_allow_toast
            else
                R.string.su_deny_toast

            Utils.toast(context.getString(resId, policy.appName), Toast.LENGTH_SHORT)
        }
    }
}
