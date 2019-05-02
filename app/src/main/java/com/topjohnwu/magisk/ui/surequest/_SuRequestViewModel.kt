package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.PackageManager
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.data.database.MagiskDB
import com.topjohnwu.magisk.model.entity.Policy
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.SuDialogEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.SuConnector
import com.topjohnwu.magisk.utils.SuLogger
import com.topjohnwu.magisk.utils.feature.WIP
import com.topjohnwu.magisk.utils.now
import io.reactivex.Single
import timber.log.Timber
import java.util.concurrent.TimeUnit.MILLISECONDS
import java.util.concurrent.TimeUnit.MINUTES

@WIP
class _SuRequestViewModel(
    intent: Intent,
    action: String,
    private val packageManager: PackageManager,
    private val database: MagiskDB
) : MagiskViewModel() {

    private val connector: Single<SuConnector> = Single.fromCallable {
        val socketName = intent.extras?.getString("socket") ?: let {
            deny()
            throw IllegalStateException("Socket is empty or null")
        }
        object : SuConnector(socketName) {
            override fun onResponse() {
                policy.subscribeK { out.writeInt(it.policy) } //this just might be incorrect, lol
            }
        } as SuConnector
    }.cache()

    private val policy: Single<Policy> = connector.map {
        val bundle = it.readSocketInput() ?: throw IllegalStateException("Socket bundle is null")
        val uid = bundle.getString("uid")?.toIntOrNull() ?: let {
            deny()
            throw IllegalStateException("UID is empty or null")
        }
        database.clearOutdated()
        database.getPolicy(uid) ?: Policy(uid, packageManager)
    }.cache()

    init {
        when (action) {
            SuRequestActivity.LOG -> SuLogger.handleLogs(intent).also { die() }
            SuRequestActivity.NOTIFY -> SuLogger.handleNotify(intent).also { die() }
            SuRequestActivity.REQUEST -> process()
            else -> back() // invalid action, should ignore
        }
    }

    private fun process() {
        policy.subscribeK(onError = ::deny) { process(it) }
    }

    private fun process(policy: Policy) {
        if (policy.packageName == BuildConfig.APPLICATION_ID)
            deny().also { return }

        if (policy.policy != Policy.INTERACTIVE)
            grant().also { return }

        when (Config.get<Int>(Config.Key.SU_AUTO_RESPONSE)) {
            Config.Value.SU_AUTO_DENY -> deny().also { return }
            Config.Value.SU_AUTO_ALLOW -> grant().also { return }
        }

        requestDialog(policy)
    }

    fun deny(e: Throwable? = null) = updatePolicy(Policy.DENY, 0).also { Timber.e(e) }
    fun grant(time: Long = 0) = updatePolicy(Policy.ALLOW, time)

    private fun updatePolicy(action: Int, time: Long) {

        fun finish(e: Throwable? = null) = die().also { Timber.e(e) }

        policy
            .map { it.policy = action; it }
            .doOnSuccess {
                if (time >= 0) {
                    it.until = if (time == 0L) {
                        0
                    } else {
                        MILLISECONDS.toSeconds(now) + MINUTES.toSeconds(time)
                    }
                    database.updatePolicy(it)
                }
            }
            .flatMap { connector }
            .subscribeK(onError = ::finish) {
                it.response()
                finish()
            }
    }

    private fun requestDialog(policy: Policy) {
        SuDialogEvent(policy).publish()
    }

    private fun die() = DieEvent().publish()

}