package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.SharedPreferences
import android.graphics.drawable.Drawable
import android.os.CountDownTimer
import androidx.core.content.edit
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
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import java.util.concurrent.TimeUnit.SECONDS

class SuRequestViewModel(
    policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences
) : BaseViewModel() {

    data class UiState(
        val showUi: Boolean = false,
        val icon: Drawable? = null,
        val title: String = "",
        val packageName: String = "",
        val isSharedUid: Boolean = false,
        val selectedItemPosition: Int = 0,
        val grantEnabled: Boolean = false,
        val denyCountdown: Int = 0,
        val useTapjackProtection: Boolean = false,
    )

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    var authenticate: (onSuccess: () -> Unit) -> Unit = { it() }
    var finishActivity: () -> Unit = {}

    val useTapjackProtection get() = _uiState.value.useTapjackProtection

    private val handler = SuRequestHandler(AppContext.packageManager, policyDB)
    private val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
    private var timer = SuTimer(millis, 1000)
    private var initialized = false
    private var responded = false

    fun setSelectedItemPosition(position: Int) {
        _uiState.update { it.copy(selectedItemPosition = position) }
    }

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

        val isSharedUid = info.sharedUserId != null
        val icon: Drawable?
        val title: String
        val packageName: String

        if (app == null) {
            icon = pm.defaultActivityIcon
            title = info.sharedUserId.toString()
            packageName = info.sharedUserId.toString()
        } else {
            icon = app.loadIcon(pm)
            title = app.getLabel(pm)
            packageName = info.packageName
        }

        val selectedPos = timeoutPrefs.getInt(packageName, 0)
        _uiState.update {
            it.copy(
                showUi = true,
                icon = icon,
                title = title,
                packageName = packageName,
                isSharedUid = isSharedUid,
                selectedItemPosition = selectedPos,
                useTapjackProtection = Config.suTapjack,
                grantEnabled = false,
            )
        }
        viewModelScope.launch {
            delay(1000)
            _uiState.update { it.copy(grantEnabled = true) }
        }
        timer.start()
        initialized = true
    }

    fun activityDestroyed() {
        if (initialized && !responded) {
            respond(DENY)
        }
    }

    override fun onCleared() {
        super.onCleared()
        timer.cancel()
        if (initialized && !responded) {
            responded = true
            val pos = _uiState.value.selectedItemPosition
            runBlocking(Dispatchers.IO) {
                handler.respond(DENY, Config.Value.TIMEOUT_LIST[pos])
            }
        }
    }

    private fun respond(action: Int) {
        if (!initialized || responded) return
        responded = true
        timer.cancel()

        val pos = _uiState.value.selectedItemPosition
        val pkg = _uiState.value.packageName
        timeoutPrefs.edit { putInt(pkg, pos) }

        viewModelScope.launch(Dispatchers.IO) {
            handler.respond(action, Config.Value.TIMEOUT_LIST[pos])
            withContext(Dispatchers.Main) {
                finishActivity()
            }
        }
    }

    private fun cancelTimer() {
        timer.cancel()
        _uiState.update { it.copy(denyCountdown = 0) }
    }

    private inner class SuTimer(
        millis: Long,
        interval: Long
    ) : CountDownTimer(millis, interval) {

        override fun onTick(remains: Long) {
            _uiState.update {
                it.copy(
                    denyCountdown = (remains / 1000).toInt() + 1
                )
            }
        }

        override fun onFinish() {
            _uiState.update { it.copy(denyCountdown = 0) }
            respond(DENY)
        }
    }
}
