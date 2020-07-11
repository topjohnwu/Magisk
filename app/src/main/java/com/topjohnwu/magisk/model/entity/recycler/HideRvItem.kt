package com.topjohnwu.magisk.model.entity.recycler

import android.view.View
import android.view.ViewGroup
import androidx.databinding.ObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.ktx.addOnPropertyChangedCallback
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.ktx.toggle
import com.topjohnwu.magisk.ktx.value
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.ui.hide.HideViewModel
import kotlin.math.roundToInt

class HideItem(val item: ProcessHideApp) : ComparableRvItem<HideItem>() {

    override val layoutRes = R.layout.item_hide_md2

    val packageName = item.info.info.packageName.orEmpty()
    val items = item.processes.map { HideProcessItem(it) }

    val isExpanded = ObservableField(false)
    val itemsChecked = ObservableField(0)
    val itemsCheckedPercent = Observer(itemsChecked) {
        (itemsChecked.value.toFloat() / items.size * 100).roundToInt()
    }

    /** [toggle] depends on this functionality */
    private val isHidden get() = itemsChecked.value == items.size

    init {
        items.forEach { it.isHidden.addOnPropertyChangedCallback { recalculateChecked() } }
        recalculateChecked()
    }

    fun collapse(v: View) {
        (v.parent.parent as? ViewGroup)?.startAnimations()
        isExpanded.value = false
    }

    fun toggle(v: View) {
        (v.parent as? ViewGroup)?.startAnimations()
        isExpanded.toggle()
    }

    fun toggle(viewModel: HideViewModel): Boolean {
        // contract implies that isHidden == all checked
        if (!isHidden) {
            items.filterNot { it.isHidden.value }
        } else {
            items
        }.forEach { it.toggle(viewModel) }
        return true
    }

    private fun recalculateChecked() {
        itemsChecked.value = items.count { it.isHidden.value }
    }

    override fun contentSameAs(other: HideItem): Boolean = item == other.item
    override fun itemSameAs(other: HideItem): Boolean = item.info == other.item.info

}

class HideProcessItem(val item: StatefulProcess) : ComparableRvItem<HideProcessItem>() {

    override val layoutRes = R.layout.item_hide_process_md2

    val isHidden = ObservableField(item.isHidden)

    fun toggle(viewModel: HideViewModel) {
        isHidden.toggle()
        viewModel.toggleItem(this)
    }

    override fun contentSameAs(other: HideProcessItem) = item == other.item
    override fun itemSameAs(other: HideProcessItem) = item.name == other.item.name

}
