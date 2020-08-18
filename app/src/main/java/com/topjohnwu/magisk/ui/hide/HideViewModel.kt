package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.Queryable
import com.topjohnwu.magisk.arch.filterableListOf
import com.topjohnwu.magisk.arch.itemBindingOf
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideAppTarget
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.entity.recycler.HideItem
import com.topjohnwu.magisk.model.entity.recycler.HideProcessItem
import com.topjohnwu.magisk.utils.set
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class HideViewModel(
    private val magiskRepo: MagiskRepository
) : BaseViewModel(), Queryable {

    override val queryDelay = 1000L

    @get:Bindable
    var isShowSystem = Config.showSystemApp
        set(value) = set(value, field, { field = it }, BR.showSystem){
            Config.showSystemApp = it
            submitQuery()
        }

    @get:Bindable
    var query = ""
        set(value) = set(value, field, { field = it }, BR.query){
            submitQuery()
        }

    val items = filterableListOf<HideItem>()
    val itemBinding = itemBindingOf<HideItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<HideProcessItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = viewModelScope.launch {
        state = State.LOADING
        val apps = magiskRepo.fetchApps()
        val hides = magiskRepo.fetchHideTargets()
        val (appList, diff) = withContext(Dispatchers.Default) {
            val list = apps
                .map { createTarget(it, hides) }
                .map { HideItem(it, this@HideViewModel) }
                .sort()
            list to items.calculateDiff(list)
        }
        items.update(appList, diff)
        submitQuery()
        state = State.LOADED
    }

    // ---

    private fun createTarget(app: HideAppInfo, hideList: List<HideTarget>): HideAppTarget {
        val hidden = hideList.filter { it.packageName == app.info.packageName }
        val packageName = app.info.packageName
        val processes = app.processes.map { name ->
            StatefulProcess(name, packageName, hidden.any { name == it.process })
        }
        return HideAppTarget(app, processes)
    }

    private fun List<HideItem>.sort() = compareByDescending<HideItem> { it.itemsChecked != 0 }
        .thenBy { it.item.info.name.toLowerCase(currentLocale) }
        .thenBy { it.item.info.info.packageName }
        .let { sortedWith(it) }

    // ---

    override fun query() = items.filter {
        fun showHidden()= it.itemsChecked != 0

        fun filterSystem(): Boolean {
            return isShowSystem || it.item.info.info.flags and ApplicationInfo.FLAG_SYSTEM == 0
        }

        fun filterQuery(): Boolean {
            fun inName() = it.item.info.name.contains(query, true)
            fun inPackage() = it.item.info.info.packageName.contains(query, true)
            fun inProcesses() = it.item.processes.any { it.name.contains(query, true) }
            return inName() || inPackage() || inProcesses()
        }

        showHidden() || (filterSystem() && filterQuery())
    }

    // ---

    fun toggleItem(item: HideProcessItem) {
        magiskRepo.toggleHide(item.isHidden, item.item.packageName, item.item.name)
    }

    fun resetQuery() {
        query = ""
    }
}

