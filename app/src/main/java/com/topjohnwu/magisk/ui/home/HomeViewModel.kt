package com.topjohnwu.magisk.ui.home

import android.Manifest
import android.os.Build
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.download.RemoteFileService
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.*
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
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.Shell
import me.tatarka.bindingcollectionadapter2.BR
import kotlin.math.roundToInt

enum class MagiskState {
    NOT_INSTALLED, UP_TO_DATE, OBSOLETE, LOADING
}

class HomeViewModel(
    private val repoMagisk: MagiskRepository
) : BaseViewModel() {

    val isNoticeVisible = KObservableField(Config.safetyNotice)

    val stateMagisk = KObservableField(MagiskState.LOADING)
    val stateManager = KObservableField(MagiskState.LOADING)

    val stateMagiskRemoteVersion = KObservableField(R.string.loading.res())
    val stateMagiskInstalledVersion get() =
        "${Info.env.magiskVersionString} (${Info.env.magiskVersionCode})"
    val stateMagiskMode get() = (if (Config.coreOnly) R.string.home_status_safe else R.string.home_status_normal).res()

    val stateManagerRemoteVersion = KObservableField(R.string.loading.res())
    val stateManagerInstalledVersion = Info.stub?.let {
        "${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE}) (${it.version})"
    } ?: "${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})"
    val statePackageName = packageName
    val stateManagerProgress = KObservableField(0)

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
                is Manager -> stateManagerProgress.value = it.first.times(100f).roundToInt()
            }
        }
    }

    override fun refresh() = repoMagisk.fetchUpdate()
        .onErrorReturn { null }
        .subscribeK { it?.updateUI() }

    private fun UpdateInfo.updateUI() {
        stateMagisk.value = when {
            !Info.env.isActive -> MagiskState.NOT_INSTALLED
            magisk.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        stateManager.value = when {
            !app.isUpdateChannelCorrect && isConnected.value -> MagiskState.NOT_INSTALLED
            app.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        stateMagiskRemoteVersion.value = "${magisk.version} (${magisk.versionCode})"
        stateManagerRemoteVersion.value = "${app.version} (${app.versionCode}) (${stub.versionCode})"

        ensureEnv()
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

    fun onMagiskPressed() = withPermissions(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    ).map { check(it);it }
        .subscribeK { HomeFragmentDirections.actionHomeFragmentToInstallFragment().publish() }
        .add()

    fun toggle(kof: KObservableField<Boolean>) = kof.toggle()

    fun hideNotice() {
        Config.safetyNotice = false
        isNoticeVisible.value = false
    }

    private fun ensureEnv() {
        val invalidStates = listOf(
            MagiskState.NOT_INSTALLED,
            MagiskState.LOADING
        )

        // Don't bother checking env when magisk is not installed, loading or already has been shown
        if (
            invalidStates.any { it == stateMagisk.value } ||
            shownDialog ||
            // don't care for emulators either
            Build.DEVICE.orEmpty().contains("generic") ||
            Build.PRODUCT.orEmpty().contains("generic")
        ) {
            return
        }

        Shell.su("env_check")
            .toSingle()
            .map { it.exec() }
            .filter { !it.isSuccess }
            .subscribeK {
                shownDialog = true
                EnvFixDialog().publish()
            }
    }

    private val MagiskJson.isObsolete
        get() = Info.env.isActive && Info.env.magiskVersionCode < versionCode
    val ManagerJson.isUpdateChannelCorrect
        get() = versionCode > 0
    val ManagerJson.isObsolete
        get() = BuildConfig.VERSION_CODE < versionCode

}
