package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.rxbus.RxBus
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem
import com.topjohnwu.magisk.model.entity.recycler.HideRvItem
import com.topjohnwu.magisk.model.events.HideProcessEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.toSingle
import com.topjohnwu.superuser.Shell
import io.reactivex.Single
import me.tatarka.bindingcollectionadapter2.OnItemBind
import timber.log.Timber

class HideViewModel(
    private val packageManager: PackageManager,
    rxBus: RxBus
) : MagiskViewModel() {

    val query = KObservableField("")
    val isShowSystem = KObservableField(false)

    private val allItems = DiffObservableList(ComparableRvItem.callback)
    val items = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = OnItemBind<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@HideViewModel)
    }

    init {
        rxBus.register<HideProcessEvent>()
            .subscribeK { toggleItem(it.item) }
            .add()

        isShowSystem.addOnPropertyChangedCallback { query() }
        query.addOnPropertyChangedCallback { query() }

        refresh()
    }

    fun refresh() {
        // fetching this for every item is nonsensical, so we add .cache() so the response is all
        // the same for every single mapped item, it only actually executes the whole thing the
        // first time around.
        val hideTargets = Shell.su("magiskhide --ls").toSingle()
            .map { it.exec().out }
            .flattenAsFlowable { it }
            .map { HideTarget(it) }
            .toList()
            .cache()

        Single.fromCallable { packageManager.getInstalledApplications(0) }
            .flattenAsFlowable { it }
            .filter { it.enabled && !blacklist.contains(it.packageName) }
            .map {
                val label = Utils.getAppLabel(it, packageManager)
                val icon = it.loadIcon(packageManager)
                HideAppInfo(it, label, icon)
            }
            .filter { it.processes.isNotEmpty() }
            .map { HideRvItem(it, hideTargets.blockingGet()) }
            .toList()
            .map { it.sortBy { it.item.info.name }; it }
            .applyViewModel(this)
            .subscribeK(onError = Timber::e) {
                allItems.update(it)
                query()
            }
            .add()
    }

    private fun query(showSystem: Boolean = isShowSystem.value, query: String = this.query.value) {
        allItems.toSingle()
            .map { it.filterIsInstance<HideRvItem>() }
            .flattenAsFlowable { it }
            .filter {
                it.item.name.contains(query, ignoreCase = true) ||
                        it.item.processes.any { it.contains(query, ignoreCase = true) }
            }
            .filter { if (showSystem) true else it.item.info.flags and ApplicationInfo.FLAG_SYSTEM == 0 }
            .toList()
            .subscribeK { items.update(it) }
            .add()
    }

    private fun toggleItem(item: HideProcessRvItem) {
        val state = if (item.isHidden.value) "add" else "rm"
        "magiskhide --%s %s %s".format(state, item.packageName, item.process)
            .let { Shell.su(it) }
            .toSingle()
            .map { it.submit() }
            .subscribeK()
    }

    companion object {
        private val blacklist = listOf(
            App.self.packageName,
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
        )
    }

}