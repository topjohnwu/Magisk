package com.topjohnwu.magisk.ui.home

import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.*
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.download.Subject.Manager
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.events.OpenInappLinkEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.events.dialog.EnvFixDialog
import com.topjohnwu.magisk.events.dialog.ManagerInstallDialog
import com.topjohnwu.magisk.events.dialog.UninstallDialog
import com.topjohnwu.magisk.ktx.await
import com.topjohnwu.magisk.ktx.packageName
import com.topjohnwu.magisk.ktx.res
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch
import me.tatarka.bindingcollectionadapter2.BR
import kotlin.math.roundToInt

enum class MagiskState {
    NOT_INSTALLED, UP_TO_DATE, OBSOLETE, LOADING
}

class HomeViewModel(
    private val svc: NetworkService
) : BaseViewModel() {

    @get:Bindable
    var isNoticeVisible = Config.safetyNotice
        set(value) = set(value, field, { field = it }, BR.noticeVisible)

    @get:Bindable
    var stateMagisk = MagiskState.LOADING
        set(value) = set(value, field, { field = it }, BR.stateMagisk, BR.showUninstall)

    @get:Bindable
    var stateManager = MagiskState.LOADING
        set(value) = set(value, field, { field = it }, BR.stateManager)

    @get:Bindable
    var magiskRemoteVersion = R.string.loading.res()
        set(value) = set(value, field, { field = it }, BR.magiskRemoteVersion)

    val magiskInstalledVersion get() =
        "${Info.env.magiskVersionString} (${Info.env.magiskVersionCode})"

    @get:Bindable
    var managerRemoteVersion = R.string.loading.res()
        set(value) = set(value, field, { field = it }, BR.managerRemoteVersion)

    val managerInstalledVersion = Info.stub?.let {
        "${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE}) (${it.version})"
    } ?: "${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})"
    val statePackageName = packageName

    @get:Bindable
    var stateManagerProgress = 0
        set(value) = set(value, field, { field = it }, BR.stateManagerProgress)

    @get:Bindable
    val showUninstall get() = Info.env.isActive && state != State.LOADING

    @get:Bindable
    val showSafetyNet get() = Info.hasGMS && isConnected.get()

    val itemBinding = itemBindingOf<IconLink> {
        it.bindExtra(BR.viewModel, this)
    }

    private var shownDialog = false

    override fun refresh() = viewModelScope.launch {
        state = State.LOADING
        svc.fetchUpdate()?.apply {
            state = State.LOADED
            stateMagisk = when {
                !Info.env.isActive -> MagiskState.NOT_INSTALLED
                magisk.isObsolete -> MagiskState.OBSOLETE
                else -> MagiskState.UP_TO_DATE
            }

            stateManager = when {
                app.isObsolete -> MagiskState.OBSOLETE
                else -> MagiskState.UP_TO_DATE
            }

            magiskRemoteVersion =
                "${magisk.version} (${magisk.versionCode})"
            managerRemoteVersion =
                "${app.version} (${app.versionCode}) (${stub.versionCode})"

            launch {
                ensureEnv()
            }
        } ?: apply { state = State.LOADING_FAILED }
        notifyPropertyChanged(BR.showUninstall)
        notifyPropertyChanged(BR.showSafetyNet)
    }

    val showTest = false

    fun onTestPressed() = object : ViewEvent(), ActivityExecutor {
        override fun invoke(activity: BaseUIActivity<*, *>) {
            /* Entry point to trigger test events within the app */
        }
    }.publish()

    fun onProgressUpdate(progress: Float, subject: Subject) {
        when (subject) {
            is Manager -> stateManagerProgress = progress.times(100f).roundToInt()
        }
    }

    fun onLinkPressed(link: String) = OpenInappLinkEvent(link).publish()

    fun onDeletePressed() = UninstallDialog().publish()

    fun onManagerPressed() = when (state) {
        State.LOADED -> ManagerInstallDialog().publish()
        State.LOADING -> SnackbarEvent(R.string.loading).publish()
        else -> SnackbarEvent(R.string.no_connection).publish()
    }

    fun onMagiskPressed() = when (state) {
        State.LOADED -> withExternalRW {
            HomeFragmentDirections.actionHomeFragmentToInstallFragment().publish()
        }
        State.LOADING -> SnackbarEvent(R.string.loading).publish()
        else -> SnackbarEvent(R.string.no_connection).publish()
    }

    fun onSafetyNetPressed() =
        HomeFragmentDirections.actionHomeFragmentToSafetynetFragment().publish()

    fun hideNotice() {
        Config.safetyNotice = false
        isNoticeVisible = false
    }

    private suspend fun ensureEnv() {
        val invalidStates = listOf(
            MagiskState.NOT_INSTALLED,
            MagiskState.LOADING
        )
        if (invalidStates.any { it == stateMagisk } || shownDialog) return

        val result = Shell.su("env_check").await()
        if (!result.isSuccess) {
            shownDialog = true
            EnvFixDialog().publish()
        }
    }

    private val MagiskJson.isObsolete
        get() = Info.env.isActive && Info.env.magiskVersionCode < versionCode
    private val ManagerJson.isObsolete
        get() = BuildConfig.VERSION_CODE < versionCode

}
