package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.rxbus.RxBus
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.toSingle
import com.topjohnwu.magisk.extensions.update
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem
import com.topjohnwu.magisk.model.entity.recycler.HideRvItem
import com.topjohnwu.magisk.model.entity.state.IndeterminateState
import com.topjohnwu.magisk.model.events.HideProcessEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import me.tatarka.bindingcollectionadapter2.OnItemBind
import timber.log.Timber

class HideViewModel(
    private val magiskRepo: MagiskRepository,
    rxBus: RxBus
) : MagiskViewModel() {

    val query = KObservableField("")
    val isShowSystem = KObservableField(false)

    private val allItems = mutableListOf<ComparableRvItem<*>>()
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
        val hideTargets = magiskRepo.fetchHideTargets().cache()

        magiskRepo.fetchApps()
            .flattenAsFlowable { it }
            .map { HideRvItem(it, hideTargets.blockingGet()) }
            .toList()
            .map {
                it.sortedWith(compareBy(
                    { it.isHiddenState.value },
                    { it.item.name.toLowerCase() },
                    { it.packageName }
                ))
            }
            .doOnSuccess { allItems.update(it) }
            .flatMap { queryRaw() }
            .applyViewModel(this)
            .subscribeK(onError = Timber::e) { items.update(it.first, it.second) }
            .add()
    }

    private fun query() = queryRaw()
        .subscribeK { items.update(it.first, it.second) }
        .add()

    private fun queryRaw(
        showSystem: Boolean = isShowSystem.value,
        query: String = this.query.value
    ) = allItems.toSingle()
        .map { it.filterIsInstance<HideRvItem>() }
        .flattenAsFlowable { it }
        .filter {
            it.item.name.contains(query, ignoreCase = true) ||
                    it.item.processes.any { it.contains(query, ignoreCase = true) }
        }
        .filter {
            showSystem || (it.isHiddenState.value != IndeterminateState.UNCHECKED) ||
                    (it.item.info.flags and ApplicationInfo.FLAG_SYSTEM == 0)
        }
        .toList()
        .map { it to items.calculateDiff(it) }

    private fun toggleItem(item: HideProcessRvItem) = magiskRepo
        .toggleHide(item.isHidden.value, item.packageName, item.process)
        .subscribeK()
        .add()

}