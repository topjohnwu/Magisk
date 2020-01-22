package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageManager
import android.os.CountDownTimer
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.toPolicy
import com.topjohnwu.magisk.extensions.now
import timber.log.Timber
import java.util.concurrent.TimeUnit

abstract class SuRequestHandler(
    private val packageManager: PackageManager,
    private val policyDB: PolicyDao
) {
    protected var timer: CountDownTimer = object : CountDownTimer(
        TimeUnit.MINUTES.toMillis(1), TimeUnit.MINUTES.toMillis(1)) {
        override fun onFinish() {
            respond(MagiskPolicy.DENY, 0)
        }
        override fun onTick(remains: Long) {}
    }
        set(value) {
            field.cancel()
            field = value
            field.start()
        }

    protected lateinit var policy: MagiskPolicy

    private val cleanupTasks = mutableListOf<() -> Unit>()
    private lateinit var connector: SuConnector

    abstract fun onStart()
    abstract fun onRespond()

    fun start(intent: Intent): Boolean {
        val socketName = intent.getStringExtra("socket") ?: return false

        try {
            connector = object : SuConnector(socketName) {
                override fun onResponse() {
                    out.writeInt(policy.policy)
                }
            }
            val map = connector.readRequest()
            val uid = map["uid"]?.toIntOrNull() ?: return false
            policy = uid.toPolicy(packageManager)
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        // Never allow com.topjohnwu.magisk (could be malware)
        if (policy.packageName == BuildConfig.APPLICATION_ID)
            return false

        when (Config.suAutoReponse) {
            Config.Value.SU_AUTO_DENY -> {
                respond(MagiskPolicy.DENY, 0)
                return true
            }
            Config.Value.SU_AUTO_ALLOW -> {
                respond(MagiskPolicy.ALLOW, 0)
                return true
            }
        }

        timer.start()
        cleanupTasks.add {
            timer.cancel()
        }

        onStart()
        return true
    }

    private fun respond() {
        connector.response()
        cleanupTasks.forEach { it() }
        onRespond()
    }

    fun respond(action: Int, time: Int) {
        val until = if (time > 0)
            TimeUnit.MILLISECONDS.toSeconds(now) + TimeUnit.MINUTES.toSeconds(time.toLong())
        else
            time.toLong()

        policy.policy = action
        policy.until = until
        policy.uid = policy.uid % 100000 + Const.USER_ID * 100000

        if (until >= 0)
            policyDB.update(policy).blockingAwait()

        respond()
    }
}
