package com.topjohnwu.magisk.model.entity.recycler

import android.view.View
import android.view.ViewGroup
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.ui.hide.HideViewModel
import com.topjohnwu.magisk.utils.addOnPropertyChangedCallback
import com.topjohnwu.magisk.utils.set
import kotlin.math.roundToInt

class HideItem(val item: ProcessHideApp) : ObservableItem<HideItem>() {

    override val layoutRes = R.layout.item_hide_md2

    val packageName = item.info.info.packageName.orEmpty()
    val items = item.processes.map { HideProcessItem(it) }

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    @get:Bindable
    var itemsChecked = 0
        set(value) = set(value, field, { field = it }, BR.itemsChecked, BR.itemsCheckedPercent)

    @get:Bindable
    val itemsCheckedPercent get() = (itemsChecked.toFloat() / items.size * 100).roundToInt()

    private val isHidden get() = itemsChecked == items.size

    init {
        items.forEach { it.addOnPropertyChangedCallback(BR.hidden) { recalculateChecked() } }
        recalculateChecked()
    }

    fun collapse(v: View) {
        (v.parent.parent as? ViewGroup)?.startAnimations()
        isExpanded = false
    }

    fun toggle(v: View) {
        (v.parent as? ViewGroup)?.startAnimations()
        isExpanded = !isExpanded
    }

    fun toggle(viewModel: HideViewModel): Boolean {
        // contract implies that isHidden == all checked
        if (!isHidden) {
            items.filterNot { it.isHidden }
        } else {
            items
        }.forEach { it.toggle(viewModel) }
        return true
    }

    private fun recalculateChecked() {
        itemsChecked = items.count { it.isHidden }
    }

    override fun contentSameAs(other: HideItem): Boolean = item == other.item
    override fun itemSameAs(other: HideItem): Boolean = item.info == other.item.info

}

class HideProcessItem(val item: StatefulProcess) : ObservableItem<HideProcessItem>() {

    override val layoutRes = R.layout.item_hide_process_md2

    @get:Bindable
    var isHidden = item.isHidden
        set(value) = set(value, field, { field = it }, BR.hidden)


    fun toggle(viewModel: HideViewModel) {
        isHidden = !isHidden
        viewModel.toggleItem(this)
    }

    override fun contentSameAs(other: HideProcessItem) = item == other.item
    override fun itemSameAs(other: HideProcessItem) = item.name == other.item.name

}
