package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.Intent
import android.os.Process
import android.widget.Toast
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.PolicyDao
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.entity.toLog
import com.topjohnwu.magisk.model.entity.toPolicy
import java.util.*

object SuLogger {

    fun handleLogs(context: Context, intent: Intent) {

        val fromUid = intent.getIntExtra("from.uid", -1)
        if (fromUid < 0) return
        if (fromUid == Process.myUid()) return

        val pm = context.packageManager

        val notify: Boolean
        val data = intent.extras
        val policy: MagiskPolicy = if (data!!.containsKey("notify")) {
            notify = data.getBoolean("notify")
            runCatching {
                fromUid.toPolicy(pm)
            }.getOrElse { return }
        } else {
            // Doesn't report whether notify or not, check database ourselves
            val policyDB = get<PolicyDao>()
            val policy = policyDB.fetch(fromUid).blockingGet() ?: return
            notify = policy.notification
            policy
        }.copy(policy = data.getInt("policy", -1))

        if (policy.policy < 0)
            return

        if (notify)
            handleNotify(context, policy)

        val toUid = intent.getIntExtra("to.uid", -1)
        if (toUid < 0) return

        val pid = intent.getIntExtra("pid", -1)
        if (pid < 0) return

        val command = intent.getStringExtra("command") ?: return
        val log = policy.toLog(
            toUid = toUid,
            fromPid = pid,
            command = command,
            date = Date()
        )

        val logRepo = get<LogRepository>()
        logRepo.put(log).blockingGet()?.printStackTrace()
    }

    private fun handleNotify(context: Context, policy: MagiskPolicy) {
        if (policy.notification && Config.suNotification == Config.Value.NOTIFICATION_TOAST) {
            Utils.toast(
                context.getString(
                    if (policy.policy == MagiskPolicy.ALLOW)
                        R.string.su_allow_toast
                    else
                        R.string.su_deny_toast, policy.appName
                ),
                Toast.LENGTH_SHORT
            )
        }
    }

    fun handleNotify(context: Context, intent: Intent) {
        val fromUid = intent.getIntExtra("from.uid", -1)
        if (fromUid < 0) return
        if (fromUid == Process.myUid()) return
        runCatching {
            val pm = context.packageManager
            val policy = fromUid.toPolicy(pm)
                .copy(policy = intent.getIntExtra("policy", -1))
            if (policy.policy >= 0)
                handleNotify(context, policy)
        }
    }
}
