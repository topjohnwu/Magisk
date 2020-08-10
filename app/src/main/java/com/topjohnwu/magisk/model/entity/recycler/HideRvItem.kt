package com.topjohnwu.magisk.model.entity.recycler

import android.view.View
import android.view.ViewGroup
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.model.entity.HideAppTarget
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.ui.hide.HideViewModel
import com.topjohnwu.magisk.utils.addOnPropertyChangedCallback
import com.topjohnwu.magisk.utils.set
import kotlin.math.roundToInt

class HideItem(
    val item: HideAppTarget,
    viewModel: HideViewModel
) : ObservableItem<HideItem>() {

    override val layoutRes = R.layout.item_hide_md2

    val packageName = item.info.info.packageName.orEmpty()
    val items = item.processes.map { HideProcessItem(it, viewModel) }

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    var itemsChecked = 0
        set(value) = set(value, field, { field = it }, BR.itemsCheckedPercent)

    @get:Bindable
    val itemsCheckedPercent get() = (itemsChecked.toFloat() / items.size * 100).roundToInt()

    private var state: Boolean? = false
        set(value) = set(value, field, { field = it }, BR.hiddenState)

    @get:Bindable
    var hiddenState: Boolean?
        get() = state
        set(value) = set(value, state, { state = it }, BR.hiddenState) {
            if (value == true) {
                items.filterNot { it.isHidden }
            } else {
                items
            }.forEach { it.toggle() }
        }

    init {
        items.forEach { it.addOnPropertyChangedCallback(BR.hidden) { recalculateChecked() } }
        recalculateChecked()
    }

    fun toggleExpand(v: View) {
        (v.parent as? ViewGroup)?.startAnimations()
        isExpanded = !isExpanded
    }

    private fun recalculateChecked() {
        itemsChecked = items.count { it.isHidden }
        state = when (itemsChecked) {
            0 -> false
            items.size -> true
            else -> null
        }
    }

    override fun contentSameAs(other: HideItem): Boolean = item == other.item
    override fun itemSameAs(other: HideItem): Boolean = item.info == other.item.info

}

class HideProcessItem(
    val item: StatefulProcess,
    val viewModel: HideViewModel
) : ObservableItem<HideProcessItem>() {

    override val layoutRes = R.layout.item_hide_process_md2

    @get:Bindable
    var isHidden = item.isHidden
        set(value) = set(value, field, { field = it }, BR.hidden) {
            viewModel.toggleItem(this)
        }

    fun toggle() {
        isHidden = !isHidden
    }

    override fun contentSameAs(other: HideProcessItem) = item == other.item
    override fun itemSameAs(other: HideProcessItem) = item.name == other.item.name

}
