package com.topjohnwu.magisk.redesign.home

import android.Manifest
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.download.RemoteFileService
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Magisk
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Manager
import com.topjohnwu.magisk.model.entity.recycler.DeveloperItem
import com.topjohnwu.magisk.model.entity.recycler.HomeItem
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.model.events.dialog.EnvFixDialog
import com.topjohnwu.magisk.model.events.dialog.ManagerInstallDialog
import com.topjohnwu.magisk.model.events.dialog.UninstallDialog
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.ui.home.MagiskState
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.Shell
import me.tatarka.bindingcollectionadapter2.BR
import me.tatarka.bindingcollectionadapter2.ItemBinding
import me.tatarka.bindingcollectionadapter2.OnItemBind
import kotlin.math.roundToInt

class HomeViewModel(
    private val repoMagisk: MagiskRepository
) : CompatViewModel() {

    val stateMagisk = KObservableField(MagiskState.LOADING)
    val stateManager = KObservableField(MagiskState.LOADING)
    val stateTextMagisk = Observer(stateMagisk) {
        when (stateMagisk.value) {
            MagiskState.NOT_INSTALLED -> R.string.installed_error_md2.res()
            MagiskState.UP_TO_DATE -> R.string.up_to_date_md2.res()
            MagiskState.LOADING -> R.string.loading_md2.res()
            MagiskState.OBSOLETE -> R.string.obsolete_md2.res()
        }
    }
    val stateTextManager = Observer(stateManager) {
        when (stateManager.value) {
            MagiskState.NOT_INSTALLED -> R.string.channel_error_md2.res()
            MagiskState.UP_TO_DATE -> R.string.up_to_date_md2.res()
            MagiskState.LOADING -> R.string.loading_md2.res()
            MagiskState.OBSOLETE -> R.string.obsolete_md2.res()
        }
    }
    val statePackageManager = packageName
    val statePackageOriginal = statePackageManager == BuildConfig.APPLICATION_ID
    val stateVersionUpdateMagisk = KObservableField("")
    val stateVersionUpdateManager = KObservableField("")

    val stateMagiskProgress = KObservableField(0)
    val stateManagerProgress = KObservableField(0)

    val stateMagiskExpanded = KObservableField(false)
    val stateManagerExpanded = KObservableField(false)
    val stateDeviceExpanded = KObservableField(false)

    val stateHideManagerName = R.string.manager.res().let {
        if (!statePackageOriginal) {
            it.replaceRandomWithSpecial(3)
        } else {
            it
        }
    }

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
                is Magisk.Download,
                is Magisk.Flash -> stateMagiskProgress.value = it.first.times(100f).roundToInt()
                is Manager -> stateManagerProgress.value = it.first.times(100f).roundToInt()
            }
        }
    }

    override fun refresh() = repoMagisk.fetchUpdate()
        .onErrorReturn { Info.remote }
        .subscribeK { updateBy(it) }

    private fun updateBy(info: UpdateInfo) {
        stateMagisk.value = when {
            !info.magisk.isInstalled -> MagiskState.NOT_INSTALLED
            info.magisk.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        stateManager.value = when {
            !info.app.isUpdateChannelCorrect && isConnected.value -> MagiskState.NOT_INSTALLED
            info.app.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        stateVersionUpdateMagisk.value = when {
            info.magisk.isObsolete -> "%s > %s".format(
                Info.magiskVersionString.clipVersion(),
                info.magisk.version.clipVersion()
            )
            else -> ""
        }

        stateVersionUpdateManager.value = when {
            info.app.isObsolete -> "%s > %s".format(
                BuildConfig.VERSION_NAME.clipVersion(),
                info.app.version.clipVersion()
            )
            else -> ""
        }

        ensureEnv()
    }

    fun onLinkPressed(link: String) = OpenInappLinkEvent(link).publish()

    fun onDeletePressed() = UninstallDialog().publish()

    fun onManagerPressed() = ManagerInstallDialog().publish()

    fun onMagiskPressed() = withPermissions(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    ).map { check(it);it }
        .subscribeK { Navigation.install().publish() }
        .add()

    fun toggle(kof: KObservableField<Boolean>) = kof.toggle()

    private fun ensureEnv() {
        val invalidStates = listOf(
            MagiskState.NOT_INSTALLED,
            MagiskState.LOADING
        )

        // Don't bother checking env when magisk is not installed, loading or already has been shown
        if (invalidStates.any { it == stateMagisk.value } || shownDialog) {
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

}

@Suppress("unused")
val MagiskJson.isInstalled
    get() = Info.magiskVersionCode > 0
val MagiskJson.isObsolete
    get() = Info.magiskVersionCode < versionCode && isInstalled
val ManagerJson.isUpdateChannelCorrect
    get() = versionCode > 0
val ManagerJson.isObsolete
    get() = BuildConfig.VERSION_CODE < versionCode

fun String.clipVersion() = substringAfter('-')

inline fun <T : ComparableRvItem<T>> itemBindingOf(
    crossinline body: (ItemBinding<*>) -> Unit = {}
) = OnItemBind<T> { itemBinding, _, item ->
    item.bind(itemBinding)
    body(itemBinding)
}