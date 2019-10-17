package com.topjohnwu.magisk.redesign.home

import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.magisk.model.entity.recycler.HomeItem
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.model.events.dialog.EnvFixDialog
import com.topjohnwu.magisk.model.events.dialog.MagiskInstallDialog
import com.topjohnwu.magisk.model.events.dialog.ManagerInstallDialog
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.ui.home.MagiskState
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.Shell
import me.tatarka.bindingcollectionadapter2.BR
import me.tatarka.bindingcollectionadapter2.ItemBinding
import me.tatarka.bindingcollectionadapter2.OnItemBind

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

    val stateHideManagerName = R.string.manager.res().let {
        if (!statePackageOriginal) {
            it.replaceRandomWithSpecial(3)
        } else {
            it
        }
    }

    val itemsMainline =
        listOf(HomeItem.PayPal.Mainline, HomeItem.Patreon, HomeItem.Twitter.Mainline)
    val itemsApp =
        listOf(HomeItem.PayPal.App, HomeItem.Twitter.App)
    val itemsProject =
        listOf(HomeItem.Github, HomeItem.Xda)
    val itemBinding = itemBindingOf<HomeItem> {
        it.bindExtra(BR.viewModel, this)
    }

    private var shownDialog = false

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

    fun onDeletePressed() {}

    fun onManagerPressed() = ManagerInstallDialog().publish()

    fun onMagiskPressed() = MagiskInstallDialog().publish()

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