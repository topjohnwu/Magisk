package com.topjohnwu.magisk.redesign.hide

import android.content.pm.ApplicationInfo
import android.view.MenuItem
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.entity.recycler.HideItem
import com.topjohnwu.magisk.model.entity.recycler.HideProcessItem
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.FilterableDiffObservableList
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.currentLocale

class HideViewModel(
    private val magiskRepo: MagiskRepository
) : CompatViewModel() {

    var isShowSystem = false
        set(value) {
            field = value
            query()
        }

    val query = KObservableField("")
    val items = filterableListOf<HideItem>()
    val itemBinding = itemBindingOf<HideItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<HideProcessItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = magiskRepo.fetchApps()
        .map { it to magiskRepo.fetchHideTargets().blockingGet() }
        .map { pair -> pair.first.map { mergeAppTargets(it, pair.second) } }
        .flattenAsFlowable { it }
        .map { HideItem(it) }
        .toList()
        .map { it.sort() }
        .map { it to items.calculateDiff(it) }
        .subscribeK {
            items.update(it.first, it.second)
            query()
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

    private fun query(
        query: String = this.query.value,
        showSystem: Boolean = isShowSystem
    ) = items.filter {
        fun filterSystem(): Boolean {
            return showSystem || it.item.info.info.flags and ApplicationInfo.FLAG_SYSTEM == 0
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

    fun menuItemPressed(menuItem: MenuItem) = when (menuItem.itemId) {
        R.id.action_show_system -> isShowSystem = (!menuItem.isChecked)
            .also { menuItem.isChecked = it }
        else -> null
    }?.let { true } ?: false

    fun toggleItem(item: HideProcessItem) = magiskRepo
        .toggleHide(item.isHidden.value, item.item.packageName, item.item.name)
        // might wanna reorder the list to display the item at the top
        .subscribeK()
        .add()

}

inline fun <T : ComparableRvItem<T>> filterableListOf(
    vararg newItems: T
) = FilterableDiffObservableList(object : DiffObservableList.Callback<T> {
    override fun areItemsTheSame(oldItem: T, newItem: T) = oldItem.genericItemSameAs(newItem)
    override fun areContentsTheSame(oldItem: T, newItem: T) = oldItem.genericContentSameAs(newItem)
}).also { it.update(newItems.toList()) }