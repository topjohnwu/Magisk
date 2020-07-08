package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import androidx.databinding.Bindable
import androidx.databinding.ObservableField
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.value
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.entity.recycler.HideItem
import com.topjohnwu.magisk.model.entity.recycler.HideProcessItem
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.Queryable
import com.topjohnwu.magisk.ui.base.filterableListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf

class HideViewModel(
    private val magiskRepo: MagiskRepository
) : BaseViewModel(), Queryable {

    override val queryDelay = 1000L

    var isShowSystem = false
        @Bindable get
        set(value) {
            field = value
            notifyPropertyChanged(BR.showSystem)
            submitQuery()
        }

    var query = ""
        @Bindable get
        set(value) {
            field = value
            notifyPropertyChanged(BR.query)
            submitQuery()
        }
    val items = filterableListOf<HideItem>()
    val itemBinding = itemBindingOf<HideItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<HideProcessItem> {
        it.bindExtra(BR.viewModel, this)
    }

    val isFilterExpanded = ObservableField(false)

    override fun rxRefresh() = magiskRepo.fetchApps()
        .map { it to magiskRepo.fetchHideTargets().blockingGet() }
        .map { pair -> pair.first.map { mergeAppTargets(it, pair.second) } }
        .flattenAsFlowable { it }
        .map { HideItem(it) }
        .toList()
        .map { it.sort() }
        .map { it to items.calculateDiff(it) }
        .applyViewModel(this)
        .subscribeK {
            items.update(it.first, it.second)
            submitQuery()
        }

    // ---

    private fun mergeAppTargets(a: HideAppInfo, ts: List<HideTarget>): ProcessHideApp {
        val relevantTargets = ts.filter { it.packageName == a.info.packageName }
        val packageName = a.info.packageName
        val processes = a.processes
            .map { StatefulProcess(it, packageName, relevantTargets.any { i -> it == i.process }) }
        return ProcessHideApp(a, processes)
    }

    private fun List<HideItem>.sort() = compareByDescending<HideItem> { it.itemsChecked.value }
        .thenBy { it.item.info.name.toLowerCase(currentLocale) }
        .thenBy { it.item.info.info.packageName }
        .let { sortedWith(it) }

    // ---

    override fun query() = items.filter {
        fun filterSystem(): Boolean {
            return isShowSystem || it.item.info.info.flags and ApplicationInfo.FLAG_SYSTEM == 0
        }

        fun filterQuery(): Boolean {
            val inName = it.item.info.name.contains(query, true)
            val inPackage = it.item.info.info.packageName.contains(query, true)
            val inProcesses = it.item.processes.any { it.name.contains(query, true) }
            return inName || inPackage || inProcesses
        }

        filterSystem() && filterQuery()
    }

    // ---

    fun toggleItem(item: HideProcessItem) = magiskRepo
        .toggleHide(item.isHidden.value, item.item.packageName, item.item.name)

    fun resetQuery() {
        query = ""
    }

    fun hideFilter() {
        isFilterExpanded.value = false
    }

}

