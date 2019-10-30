package com.topjohnwu.magisk.redesign.hide

import android.content.pm.ApplicationInfo
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.toSingle
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.entity.recycler.HideItem
import com.topjohnwu.magisk.model.entity.recycler.HideProcessItem
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem
import com.topjohnwu.magisk.model.events.HideProcessEvent
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.RxBus
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.disposables.Disposable
import java.util.*

class HideViewModel(
    private val magiskRepo: MagiskRepository,
    rxBus: RxBus
) : CompatViewModel() {

    @Volatile
    private var cache = listOf<HideItem>()
        set(value) {
            field = Collections.synchronizedList(value)
        }
    private var queryJob: Disposable? = null
        set(value) {
            field?.dispose()
            field = value
        }

    val query = KObservableField("")
    val isShowSystem = KObservableField(true)
    val items = diffListOf<HideItem>()
    val itemBinding = itemBindingOf<HideItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<HideProcessItem> {
        it.bindExtra(BR.viewModel, this)
    }

    init {
        rxBus.register<HideProcessEvent>()
            .subscribeK { toggleItem(it.item) }
            .add()
    }

    override fun refresh() = magiskRepo.fetchApps()
        .map { it to magiskRepo.fetchHideTargets().blockingGet() }
        .map { pair -> pair.first.map { mergeAppTargets(it, pair.second) } }
        .flattenAsFlowable { it }
        .map { HideItem(it) }
        .toList()
        .map { it.sort() }
        .subscribeK {
            cache = it
            queryIfNecessary()
        }

    override fun onCleared() {
        queryJob?.dispose()
        super.onCleared()
    }

    // ---

    private fun mergeAppTargets(a: HideAppInfo, ts: List<HideTarget>): ProcessHideApp {
        val relevantTargets = ts.filter { it.packageName == a.info.packageName }
        val processes = a.processes
            .map { StatefulProcess(it, relevantTargets.any { i -> it == i.process }) }
        return ProcessHideApp(a, processes)
    }

    private fun List<HideItem>.sort() = sortedWith(compareBy(
        { it.isHidden },
        { it.item.info.name.toLowerCase(currentLocale) },
        { it.item.info.info.packageName }
    ))

    // ---

    /** We don't need to re-query when the app count matches. */
    private fun queryIfNecessary() {
        if (items.size != cache.size) {
            query()
        }
    }

    private fun query(
        query: String = this.query.value,
        showSystem: Boolean = isShowSystem.value
    ) = cache.toSingle()
        .flattenAsFlowable { it }
        .parallel()
        .filter { showSystem || it.item.info.info.flags and ApplicationInfo.FLAG_SYSTEM == 0 }
        .filter {
            val inName = it.item.info.name.contains(query, true)
            val inPackage = it.item.info.info.packageName.contains(query, true)
            val inProcesses = it.item.processes.any { it.name.contains(query, true) }
            inName || inPackage || inProcesses
        }
        .sequential()
        .toList()
        .map { it to items.calculateDiff(it) }
        .subscribeK { items.update(it.first, it.second) }
        .let { queryJob = it }

    // ---

    private fun toggleItem(item: HideProcessRvItem) = magiskRepo
        .toggleHide(item.isHidden.value, item.packageName, item.process)
        .subscribeK()
        .add()

}