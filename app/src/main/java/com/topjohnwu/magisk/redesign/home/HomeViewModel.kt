package com.topjohnwu.magisk.redesign.home

import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.res
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.magisk.model.entity.recycler.HomeItem
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.ui.home.MagiskState
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

    val itemsMainline =
        listOf(HomeItem.PayPal.Mainline, HomeItem.Patreon, HomeItem.Twitter.Mainline)
    val itemsApp =
        listOf(HomeItem.PayPal.App, HomeItem.Twitter.App)
    val itemsProject =
        listOf(HomeItem.Github, HomeItem.Xda)
    val itemBinding = itemBindingOf<HomeItem> {
        it.bindExtra(BR.viewModel, this)
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
    }

    fun onDeletePressed() {}
    fun onLinkPressed(link: String) = OpenInappLinkEvent(link).publish()

}

@Suppress("unused")
val MagiskJson.isInstalled
    get() = Info.magiskVersionCode > 0
val MagiskJson.isObsolete
    get() = Info.magiskVersionCode < versionCode
val ManagerJson.isUpdateChannelCorrect
    get() = versionCode > 0
val ManagerJson.isObsolete
    get() = BuildConfig.VERSION_CODE < versionCode

inline fun <T : ComparableRvItem<T>> itemBindingOf(
    crossinline body: (ItemBinding<*>) -> Unit = {}
) = OnItemBind<T> { itemBinding, _, item ->
    item.bind(itemBinding)
    body(itemBinding)
}