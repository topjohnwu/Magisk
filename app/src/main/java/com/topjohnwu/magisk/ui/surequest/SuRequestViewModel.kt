package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.content.res.Resources
import android.graphics.drawable.Drawable
import android.os.CountDownTimer
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.ALLOW
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.DENY
import com.topjohnwu.magisk.core.su.SuRequestHandler
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.events.DieEvent
import com.topjohnwu.magisk.events.ShowUIEvent
import com.topjohnwu.magisk.events.dialog.BiometricEvent
import com.topjohnwu.magisk.ui.superuser.SpinnerRvItem
import com.topjohnwu.magisk.utils.set
import kotlinx.coroutines.launch
import me.tatarka.bindingcollectionadapter2.BindingListViewAdapter
import me.tatarka.bindingcollectionadapter2.ItemBinding
import java.util.concurrent.TimeUnit.SECONDS

class SuRequestViewModel(
    pm: PackageManager,
    policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences,
    private val res: Resources
) : BaseViewModel() {

    lateinit var icon: Drawable
    lateinit var title: String
    lateinit var packageName: String

    @get:Bindable
    var denyText = res.getString(R.string.deny)
        set(value) = set(value, field, { field = it }, BR.denyText)

    @get:Bindable
    var selectedItemPosition = 0
        set(value) = set(value, field, { field = it }, BR.selectedItemPosition)

    @get:Bindable
    var grantEnabled = false
        set(value) = set(value, field, { field = it }, BR.grantEnabled)

    private val items = res.getStringArray(R.array.allow_timeout).map { SpinnerRvItem(it) }
    val adapter = BindingListViewAdapter<SpinnerRvItem>(1).apply {
        itemBinding = ItemBinding.of { binding, _, item ->
            item.bind(binding)
        }
        setItems(items)
    }

    private val handler = SuRequestHandler(pm, policyDB)
    private lateinit var timer: CountDownTimer

    fun grantPressed() {
        cancelTimer()
        if (BiometricHelper.isEnabled) {
            BiometricEvent {
                onSuccess {
                    respond(ALLOW)
                }
            }.publish()
        } else {
            respond(ALLOW)
        }
    }

    fun denyPressed() {
        respond(DENY)
    }

    fun spinnerTouched(): Boolean {
        cancelTimer()
        return false
    }

    fun handleRequest(intent: Intent) {
        viewModelScope.launch {
            if (handler.start(intent))
                showDialog(handler.policy)
            else
                DieEvent().publish()
        }
    }

    private fun showDialog(policy: SuPolicy) {
        icon = policy.icon
        title = policy.appName
        packageName = policy.packageName
        selectedItemPosition = timeoutPrefs.getInt(policy.packageName, 0)

        // Set timer
        val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
        timer = SuTimer(millis, 1000).apply { start() }

        // Actually show the UI
        ShowUIEvent().publish()
    }

    private fun respond(action: Int) {
        timer.cancel()

        val pos = selectedItemPosition
        timeoutPrefs.edit().putInt(handler.policy.packageName, pos).apply()
        handler.respond(action, Config.Value.TIMEOUT_LIST[pos])

        // Kill activity after response
        DieEvent().publish()
    }

    private fun cancelTimer() {
        timer.cancel()
        denyText = res.getString(R.string.deny)
    }

    private inner class SuTimer(
        private val millis: Long,
        interval: Long
    ) : CountDownTimer(millis, interval) {

        override fun onTick(remains: Long) {
            if (!grantEnabled && remains <= millis - 1000) {
                grantEnabled = true
            }
            denyText = "${res.getString(R.string.deny)} (${(remains / 1000) + 1})"
        }

        override fun onFinish() {
            denyText = res.getString(R.string.deny)
            respond(DENY)
        }

    }
}
