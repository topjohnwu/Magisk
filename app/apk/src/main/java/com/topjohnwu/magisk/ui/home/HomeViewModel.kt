package com.topjohnwu.magisk.ui.home

import android.content.ActivityNotFoundException
import android.content.Intent
import android.widget.Toast
import androidx.core.net.toUri
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.await
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.utils.asFlow
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.NonCancellable
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.topjohnwu.magisk.core.R as CoreR

class HomeViewModel(
    private val svc: NetworkService
) : AsyncLoadViewModel() {

    enum class State {
        LOADING, INVALID, OUTDATED, UP_TO_DATE
    }

    data class UiState(
        val isNoticeVisible: Boolean = Config.safetyNotice,
        val appState: State = State.LOADING,
        val managerRemoteVersion: String = "",
        val managerProgress: Int = 0,
        val showUninstall: Boolean = false,
        // The update to show in the app install dialog, as one-shot event
        val showManagerInstall: UpdateInfo? = null,
        val envFixCode: Int = 0,
        val showLoading: Boolean = false,
        val magiskState: State = computeMagiskState(),
        val magiskInstalledVersion: String = computeMagiskInstalledVersion(),
        val managerInstalledVersion: String = computeManagerInstalledVersion(),
    )

    // Info.update is reset on every connectivity change, so keep the update that the UI shows
    private var latestUpdate: UpdateInfo? = null

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    val magiskState get() = _uiState.value.magiskState
    val magiskInstalledVersion get() = _uiState.value.magiskInstalledVersion
    val managerInstalledVersion get() = _uiState.value.managerInstalledVersion

    companion object {
        private var checkedEnv = false

        fun computeMagiskState() = when {
            Info.isRooted && Info.env.isUnsupported -> State.OUTDATED
            !Info.env.isActive -> State.INVALID
            Info.env.versionCode < BuildConfig.APP_VERSION_CODE -> State.OUTDATED
            else -> State.UP_TO_DATE
        }

        fun computeMagiskInstalledVersion() = Info.env.run {
            if (isActive)
                "$versionString ($versionCode)" + if (isDebug) " (D)" else ""
            else
                ""
        }

        fun computeManagerInstalledVersion() =
            "${BuildConfig.APP_VERSION_NAME} (${BuildConfig.APP_VERSION_CODE})" +
                if (BuildConfig.DEBUG) " (D)" else ""
    }

    init {
        viewModelScope.launch {
            Info.isConnected.asFlow().collect {
                startLoading()
            }
        }
    }

    override suspend fun doLoadWork() {
        _uiState.update {
            it.copy(
                appState = State.LOADING,
                magiskState = computeMagiskState(),
                magiskInstalledVersion = computeMagiskInstalledVersion(),
                managerInstalledVersion = computeManagerInstalledVersion(),
            )
        }
        Info.fetchUpdate(svc)?.apply {
            latestUpdate = this
            val isDebug = Config.updateChannel == Config.Value.DEBUG_CHANNEL
            _uiState.update {
                it.copy(
                    appState = if (BuildConfig.APP_VERSION_CODE < versionCode) State.OUTDATED else State.UP_TO_DATE,
                    managerRemoteVersion = "$version ($versionCode)" + if (isDebug) " (D)" else ""
                )
            }
        } ?: run {
            _uiState.update { it.copy(appState = State.INVALID, managerRemoteVersion = "") }
        }
        ensureEnv()
    }

    fun onLinkPressed(link: String) {
        val intent = Intent(Intent.ACTION_VIEW, link.toUri())
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        try {
            AppContext.startActivity(intent)
        } catch (e: ActivityNotFoundException) {
            AppContext.toast(CoreR.string.open_link_failed_toast, Toast.LENGTH_SHORT)
        }
    }

    fun onDeletePressed() {
        _uiState.update { it.copy(showUninstall = true) }
    }

    fun onUninstallConsumed() {
        _uiState.update { it.copy(showUninstall = false) }
    }

    fun onManagerPressed() {
        when (_uiState.value.appState) {
            State.LOADING -> showToast(CoreR.string.loading)
            State.INVALID -> showToast(CoreR.string.no_connection)
            else -> latestUpdate?.let { info -> _uiState.update { it.copy(showManagerInstall = info) } }
                ?: showToast(CoreR.string.no_connection)
        }
    }

    fun onManagerInstallConsumed() {
        _uiState.update { it.copy(showManagerInstall = null) }
    }

    fun onEnvFixConsumed() {
        _uiState.update { it.copy(envFixCode = 0) }
    }

    // Root operations run in the ViewModel, so a configuration change can't cancel them halfway
    // and lose their result

    fun restoreImages() {
        viewModelScope.launch {
            val success = withLoading { MagiskInstaller.Restore().exec() }
            showToast(if (success) CoreR.string.restore_done else CoreR.string.restore_fail)
        }
    }

    fun fixEnv() {
        viewModelScope.launch {
            val success = withLoading { MagiskInstaller.FixEnv().exec() }
            AppContext.toast(
                if (success) CoreR.string.reboot_delay_toast else CoreR.string.setup_fail,
                Toast.LENGTH_LONG
            )
            if (success) {
                // The reboot toast has been shown; don't let leaving the app cancel it
                withContext(NonCancellable) {
                    delay(5000)
                    reboot()
                }
            }
        }
    }

    private suspend fun <R> withLoading(block: suspend () -> R): R {
        _uiState.update { it.copy(showLoading = true) }
        try {
            return block()
        } finally {
            _uiState.update { it.copy(showLoading = false) }
        }
    }

    fun hideNotice() {
        Config.safetyNotice = false
        _uiState.update { it.copy(isNoticeVisible = false) }
    }

    private suspend fun ensureEnv() {
        if (magiskState == State.INVALID || checkedEnv) return
        val cmd = "env_check ${Info.env.versionString} ${Info.env.versionCode}"
        val code = Shell.cmd(cmd).await().code
        if (code != 0) {
            _uiState.update { it.copy(envFixCode = code) }
        }
        checkedEnv = true
    }
}
