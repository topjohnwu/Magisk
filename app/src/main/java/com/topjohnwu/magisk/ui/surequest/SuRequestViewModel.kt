package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.content.res.Resources
import android.graphics.drawable.Drawable
import android.os.CountDownTimer
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy.Companion.ALLOW
import com.topjohnwu.magisk.core.model.MagiskPolicy.Companion.DENY
import com.topjohnwu.magisk.core.su.SuRequestHandler
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.model.entity.recycler.SpinnerRvItem
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.internal.UiThreadHandler
import me.tatarka.bindingcollectionadapter2.BindingListViewAdapter
import me.tatarka.bindingcollectionadapter2.ItemBinding
import java.util.concurrent.TimeUnit.SECONDS

class SuRequestViewModel(
    private val pm: PackageManager,
    private val policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences,
    private val res: Resources
) : BaseViewModel() {

    val icon = KObservableField<Drawable?>(null)
    val title = KObservableField("")
    val packageName = KObservableField("")

    val denyText = KObservableField(res.getString(R.string.deny))
    val warningText = KObservableField<CharSequence>(res.getString(R.string.su_warning))

    val selectedItemPosition = KObservableField(0)

    val grantEnabled = KObservableField(false)

    private val items = res.getStringArray(R.array.allow_timeout).map { SpinnerRvItem(it) }
    val adapter = BindingListViewAdapter<SpinnerRvItem>(1).apply {
        itemBinding = ItemBinding.of { binding, _, item ->
            item.bind(binding)
        }
        setItems(items)
    }

    private val handler = Handler()

    fun grantPressed() {
        handler.cancelTimer()
        if (BiometricHelper.isEnabled) {
            withView {
                BiometricHelper.authenticate(this) {
                    handler.respond(ALLOW)
                }
            }
        } else {
            handler.respond(ALLOW)
        }
    }

    fun denyPressed() {
        handler.respond(DENY)
    }

    fun spinnerTouched(): Boolean {
        handler.cancelTimer()
        return false
    }

    fun handleRequest(intent: Intent): Boolean {
        return handler.start(intent)
    }

    private inner class Handler : SuRequestHandler(pm, policyDB) {

        private lateinit var timer: CountDownTimer

        fun respond(action: Int) {
            timer.cancel()

            val pos = selectedItemPosition.value
            timeoutPrefs.edit().putInt(policy.packageName, pos).apply()
            respond(action, Config.Value.TIMEOUT_LIST[pos])

            // Kill activity after response
            DieEvent().publish()
        }

        fun cancelTimer() {
            timer.cancel()
            denyText.value = res.getString(R.string.deny)
        }

        override fun onStart() {
            icon.value = policy.applicationInfo.loadIcon(pm)
            title.value = policy.appName
            packageName.value = policy.packageName
            UiThreadHandler.handler.post {
                // Delay is required to properly do selection
                selectedItemPosition.value = timeoutPrefs.getInt(policy.packageName, 0)
            }

            // Set timer
            val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
            timer = SuTimer(millis, 1000).apply { start() }
        }

        private inner class SuTimer(
            private val millis: Long,
            interval: Long
        ) : CountDownTimer(millis, interval) {

            override fun onTick(remains: Long) {
                if (!grantEnabled.value && remains <= millis - 1000) {
                    grantEnabled.value = true
                }
                denyText.value = "${res.getString(R.string.deny)} (${(remains / 1000) + 1})"
            }

            override fun onFinish() {
                denyText.value = res.getString(R.string.deny)
                respond(DENY)
            }

        }
    }

}
