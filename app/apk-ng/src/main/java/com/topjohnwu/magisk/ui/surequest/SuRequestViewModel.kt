package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.SharedPreferences
import androidx.core.content.edit
import android.graphics.drawable.Drawable
import android.os.CountDownTimer
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.ktx.getLabel
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.ALLOW
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.DENY
import com.topjohnwu.magisk.core.su.SuRequestHandler
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.util.concurrent.TimeUnit.SECONDS

class SuRequestViewModel(
    policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences
) : BaseViewModel() {

    var authenticate: (onSuccess: () -> Unit) -> Unit = { it() }
    var finishActivity: () -> Unit = {}

    var icon by mutableStateOf<Drawable?>(null)
    var title by mutableStateOf("")
    var packageName by mutableStateOf("")
    var isSharedUid by mutableStateOf(false)

    var selectedItemPosition by mutableIntStateOf(0)
    var grantEnabled by mutableStateOf(false)
    var denyCountdown by mutableIntStateOf(0)

    var showUi by mutableStateOf(false)
    var useTapjackProtection by mutableStateOf(false)

    private val handler = SuRequestHandler(AppContext.packageManager, policyDB)
    private val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
    private var timer = SuTimer(millis, 1000)
    private var initialized = false

    fun grantPressed() {
        cancelTimer()
        if (Config.suAuth) {
            authenticate { respond(ALLOW) }
        } else {
            respond(ALLOW)
        }
    }

    fun denyPressed() {
        respond(DENY)
    }

    fun spinnerTouched() {
        cancelTimer()
    }

    fun handleRequest(intent: Intent) {
        viewModelScope.launch(Dispatchers.Default) {
            if (handler.start(intent))
                showDialog()
            else
                finishActivity()
        }
    }

    private fun showDialog() {
        val pm = handler.pm
        val info = handler.pkgInfo
        val app = info.applicationInfo

        isSharedUid = info.sharedUserId != null
        if (app == null) {
            icon = pm.defaultActivityIcon
            title = info.sharedUserId.toString()
            packageName = info.sharedUserId.toString()
        } else {
            icon = app.loadIcon(pm)
            title = app.getLabel(pm)
            packageName = info.packageName
        }

        selectedItemPosition = timeoutPrefs.getInt(packageName, 0)
        timer.start()
        useTapjackProtection = Config.suTapjack
        showUi = true
        initialized = true
    }

    private fun respond(action: Int) {
        if (!initialized) return
        timer.cancel()

        val pos = selectedItemPosition
        timeoutPrefs.edit { putInt(packageName, pos) }

        viewModelScope.launch {
            handler.respond(action, Config.Value.TIMEOUT_LIST[pos])
            finishActivity()
        }
    }

    private fun cancelTimer() {
        timer.cancel()
        denyCountdown = 0
    }

    private inner class SuTimer(
        private val millis: Long,
        interval: Long
    ) : CountDownTimer(millis, interval) {

        override fun onTick(remains: Long) {
            if (!grantEnabled && remains <= millis - 1000) {
                grantEnabled = true
            }
            denyCountdown = (remains / 1000).toInt() + 1
        }

        override fun onFinish() {
            denyCountdown = 0
            respond(DENY)
        }
    }

}
