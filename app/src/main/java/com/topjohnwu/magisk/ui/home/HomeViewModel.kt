package com.topjohnwu.magisk.ui.home

import android.os.Build
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.download.RemoteFileService
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.ktx.await
import com.topjohnwu.magisk.ktx.packageName
import com.topjohnwu.magisk.ktx.res
import com.topjohnwu.magisk.ktx.value
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Manager
import com.topjohnwu.magisk.model.entity.recycler.DeveloperItem
import com.topjohnwu.magisk.model.entity.recycler.HomeItem
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.model.events.dialog.EnvFixDialog
import com.topjohnwu.magisk.model.events.dialog.ManagerInstallDialog
import com.topjohnwu.magisk.model.events.dialog.UninstallDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.itemBindingOf
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch
import me.tatarka.bindingcollectionadapter2.BR
import kotlin.math.roundToInt

enum class MagiskState {
    NOT_INSTALLED, UP_TO_DATE, OBSOLETE, LOADING
}

class HomeViewModel(
    private val repoMagisk: MagiskRepository
) : BaseViewModel() {

    @get:Bindable
    var isNoticeVisible = Config.safetyNotice
        set(value) = set(value, field, { field = it }, BR.noticeVisible)

    @get:Bindable
    var stateMagisk = MagiskState.LOADING
        set(value) = set(value, field, { field = it }, BR.stateMagisk)

    @get:Bindable
    var stateManager = MagiskState.LOADING
        set(value) = set(value, field, { field = it }, BR.stateManager)

    @get:Bindable
    var magiskRemoteVersion = R.string.loading.res()
        set(value) = set(value, field, { field = it }, BR.magiskRemoteVersion)

    val magiskInstalledVersion get() =
        "${Info.env.magiskVersionString} (${Info.env.magiskVersionCode})"
    val magiskMode get() = R.string.home_status_normal.res()

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

    val items = listOf(DeveloperItem.Mainline, DeveloperItem.App, DeveloperItem.Project)
    val itemBinding = itemBindingOf<HomeItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemDeveloperBinding = itemBindingOf<DeveloperItem> {
        it.bindExtra(BR.viewModel, this)
    }

    private var shownDialog = false

    init {
        RemoteFileService.progressBroadcast.observeForever {
            when (it?.second) {
                is Manager -> stateManagerProgress = it.first.times(100f).roundToInt()
            }
        }
    }

    override fun refresh() = viewModelScope.launch {
        repoMagisk.fetchUpdate()?.apply {
            stateMagisk = when {
                !Info.env.isActive -> MagiskState.NOT_INSTALLED
                magisk.isObsolete -> MagiskState.OBSOLETE
                else -> MagiskState.UP_TO_DATE
            }

            stateManager = when {
                !app.isUpdateChannelCorrect && isConnected.value -> MagiskState.NOT_INSTALLED
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
        }
    }

    val showTest = false

    fun onTestPressed() = object : ViewEvent(), ActivityExecutor {
        override fun invoke(activity: BaseActivity) {
            /* Entry point to trigger test events within the app */
        }
    }.publish()

    fun onLinkPressed(link: String) = OpenInappLinkEvent(link).publish()

    fun onDeletePressed() = UninstallDialog().publish()

    fun onManagerPressed() = ManagerInstallDialog().publish()

    fun onMagiskPressed() = withExternalRW {
        if (it) {
            HomeFragmentDirections.actionHomeFragmentToInstallFragment().publish()
        }
    }

    fun hideNotice() {
        Config.safetyNotice = false
        isNoticeVisible = false
    }

    private suspend fun ensureEnv() {
        val invalidStates = listOf(
            MagiskState.NOT_INSTALLED,
            MagiskState.LOADING
        )

        // Don't bother checking env when magisk is not installed, loading or already has been shown
        if (
            invalidStates.any { it == stateMagisk } ||
            shownDialog ||
            // don't care for emulators either
            Build.DEVICE.orEmpty().contains("generic") ||
            Build.PRODUCT.orEmpty().contains("generic")
        ) {
            return
        }

        val result = Shell.su("env_check").await()
        if (!result.isSuccess) {
            shownDialog = true
            EnvFixDialog().publish()
        }
    }

    private val MagiskJson.isObsolete
        get() = Info.env.isActive && Info.env.magiskVersionCode < versionCode
    private val ManagerJson.isUpdateChannelCorrect
        get() = versionCode > 0
    private val ManagerJson.isObsolete
        get() = BuildConfig.VERSION_CODE < versionCode

}
