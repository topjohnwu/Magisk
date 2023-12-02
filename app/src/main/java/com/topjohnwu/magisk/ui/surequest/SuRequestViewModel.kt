package com.topjohnwu.magisk.ui.surequest

import android.annotation.SuppressLint
import android.content.Intent
import android.content.SharedPreferences
import android.content.res.Resources
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.os.CountDownTimer
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.view.accessibility.AccessibilityEvent
import android.view.accessibility.AccessibilityNodeInfo
import android.view.accessibility.AccessibilityNodeProvider
import android.widget.Toast
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.di.AppContext
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.getLabel
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.ALLOW
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.DENY
import com.topjohnwu.magisk.core.su.SuRequestHandler
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.events.AuthEvent
import com.topjohnwu.magisk.events.BiometricEvent
import com.topjohnwu.magisk.events.DieEvent
import com.topjohnwu.magisk.events.ShowUIEvent
import com.topjohnwu.magisk.utils.TextHolder
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.util.concurrent.TimeUnit.SECONDS

class SuRequestViewModel(
    policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences
) : BaseViewModel() {

    lateinit var icon: Drawable
    lateinit var title: String
    lateinit var packageName: String

    @get:Bindable
    val denyText = DenyText()

    @get:Bindable
    var selectedItemPosition = 0
        set(value) = set(value, field, { field = it }, BR.selectedItemPosition)

    @get:Bindable
    var grantEnabled = false
        set(value) = set(value, field, { field = it }, BR.grantEnabled)

    @SuppressLint("ClickableViewAccessibility")
    val grantTouchListener = View.OnTouchListener { _: View, event: MotionEvent ->
        // Filter obscured touches by consuming them.
        if (event.flags and MotionEvent.FLAG_WINDOW_IS_OBSCURED != 0
            || event.flags and MotionEvent.FLAG_WINDOW_IS_PARTIALLY_OBSCURED != 0) {
            if (event.action == MotionEvent.ACTION_UP) {
                AppContext.toast(R.string.touch_filtered_warning, Toast.LENGTH_SHORT)
            }
            return@OnTouchListener Config.suTapjack
        }
        false
    }

    private val handler = SuRequestHandler(AppContext.packageManager, policyDB)
    private val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
    private var timer = SuTimer(millis, 1000)
    private var initialized = false

    fun grantPressed() {
        cancelTimer()
        if (Config.userAuth) {
            AuthEvent { respond(ALLOW) }.publish()
        } else if (ServiceLocator.biometrics.isEnabled) {
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
        viewModelScope.launch(Dispatchers.Default) {
            if (handler.start(intent))
                showDialog()
            else
                DieEvent().publish()
        }
    }

    private fun showDialog() {
        val pm = handler.pm
        val info = handler.pkgInfo
        val app = info.applicationInfo

        if (app == null) {
            // The request is not coming from an app process, and the UID is a
            // shared UID. We have no way to know where this request comes from.
            icon = pm.defaultActivityIcon
            title = "[SharedUID] ${info.sharedUserId}"
            packageName = info.sharedUserId
        } else {
            val prefix = if (info.sharedUserId == null) "" else "[SharedUID] "
            icon = app.loadIcon(pm)
            title = "$prefix${app.getLabel(pm)}"
            packageName = info.packageName
        }

        selectedItemPosition = timeoutPrefs.getInt(packageName, 0)

        // Set timer
        timer.start()

        // Actually show the UI
        ShowUIEvent(if (Config.suTapjack) EmptyAccessibilityDelegate else null).publish()
        initialized = true
    }

    private fun respond(action: Int) {
        if (!initialized) {
            // ignore the response until showDialog done
            return
        }

        timer.cancel()

        val pos = selectedItemPosition
        timeoutPrefs.edit().putInt(packageName, pos).apply()

        viewModelScope.launch {
            handler.respond(action, Config.Value.TIMEOUT_LIST[pos])
            // Kill activity after response
            DieEvent().publish()
        }
    }

    private fun cancelTimer() {
        timer.cancel()
        denyText.seconds = 0
    }

    private inner class SuTimer(
        private val millis: Long,
        interval: Long
    ) : CountDownTimer(millis, interval) {

        override fun onTick(remains: Long) {
            if (!grantEnabled && remains <= millis - 1000) {
                grantEnabled = true
            }
            denyText.seconds = (remains / 1000).toInt() + 1
        }

        override fun onFinish() {
            denyText.seconds = 0
            respond(DENY)
        }

    }

    inner class DenyText : TextHolder() {
        var seconds = 0
            set(value) = set(value, field, { field = it }, BR.denyText)

        override fun getText(resources: Resources): CharSequence {
            return if (seconds != 0)
                "${resources.getString(R.string.deny)} ($seconds)"
            else
                resources.getString(R.string.deny)
        }
    }

    // Invisible for accessibility services
    object EmptyAccessibilityDelegate : View.AccessibilityDelegate() {
        override fun sendAccessibilityEvent(host: View, eventType: Int) {}
        override fun performAccessibilityAction(host: View, action: Int, args: Bundle?) = true
        override fun sendAccessibilityEventUnchecked(host: View, event: AccessibilityEvent) {}
        override fun dispatchPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) = true
        override fun onPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityNodeInfo(host: View, info: AccessibilityNodeInfo) {}
        override fun addExtraDataToAccessibilityNodeInfo(host: View, info: AccessibilityNodeInfo, extraDataKey: String, arguments: Bundle?) {}
        override fun onRequestSendAccessibilityEvent(host: ViewGroup, child: View, event: AccessibilityEvent): Boolean = false
        override fun getAccessibilityNodeProvider(host: View): AccessibilityNodeProvider? = null
    }
}
