package com.topjohnwu.magisk.model.entity.recycler

import android.view.View
import android.view.ViewGroup
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.ProcessHideApp
import com.topjohnwu.magisk.model.entity.StatefulProcess
import com.topjohnwu.magisk.model.entity.state.IndeterminateState
import com.topjohnwu.magisk.model.events.HideProcessEvent
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.redesign.hide.HideViewModel
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.RxBus
import kotlin.math.roundToInt

class HideItem(val item: ProcessHideApp) : ComparableRvItem<HideItem>() {

    override val layoutRes = R.layout.item_hide_md2

    val packageName = item.info.info.packageName.orEmpty()
    val items = item.processes.map { HideProcessItem(it) }

    val isExpanded = KObservableField(false)
    val itemsChecked = KObservableField(0)
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

    val isHidden = KObservableField(item.isHidden)

    fun toggle(viewModel: HideViewModel) {
        isHidden.toggle()
        viewModel.toggleItem(this)
    }

    override fun contentSameAs(other: HideProcessItem) = item == other.item
    override fun itemSameAs(other: HideProcessItem) = item.name == other.item.name

}

class HideRvItem(val item: HideAppInfo, targets: List<HideTarget>) :
    ComparableRvItem<HideRvItem>() {

    override val layoutRes: Int = R.layout.item_hide_app

    val packageName = item.info.packageName.orEmpty()
    val items = DiffObservableList(callback).also {
        val items = item.processes.map {
            val isHidden = targets.any { target ->
                packageName == target.packageName && it == target.process
            }
            HideProcessRvItem(packageName, it, isHidden)
        }
        it.update(items)
    }
    val isHiddenState = KObservableField(currentState)
    val isExpanded = KObservableField(false)

    private val itemsProcess get() = items.filterIsInstance<HideProcessRvItem>()

    private val currentState
        get() = when (itemsProcess.count { it.isHidden.value }) {
            items.size -> IndeterminateState.CHECKED
            in 1 until items.size -> IndeterminateState.INDETERMINATE
            else -> IndeterminateState.UNCHECKED
        }

    init {
        itemsProcess.forEach {
            it.isHidden.addOnPropertyChangedCallback { isHiddenState.value = currentState }
        }
    }

    fun toggle() {
        val desiredState = when (isHiddenState.value) {
            IndeterminateState.INDETERMINATE,
            IndeterminateState.UNCHECKED -> true
            IndeterminateState.CHECKED -> false
        }
        itemsProcess.forEach { it.isHidden.value = desiredState }
        isHiddenState.value = currentState
    }

    fun toggleExpansion() {
        if (items.size <= 1) return
        isExpanded.toggle()
    }

    override fun contentSameAs(other: HideRvItem): Boolean = items.all { other.items.contains(it) }
    override fun itemSameAs(other: HideRvItem): Boolean = item.info == other.item.info

}

class HideProcessRvItem(
    val packageName: String,
    val process: String,
    isHidden: Boolean
) : ComparableRvItem<HideProcessRvItem>() {

    override val layoutRes: Int = R.layout.item_hide_process

    val isHidden = KObservableField(isHidden)

    private val rxBus: RxBus by inject()

    init {
        this.isHidden.addOnPropertyChangedCallback {
            rxBus.post(HideProcessEvent(this@HideProcessRvItem))
        }
    }

    fun toggle() = isHidden.toggle()

    override fun contentSameAs(other: HideProcessRvItem): Boolean = itemSameAs(other)
    override fun itemSameAs(other: HideProcessRvItem): Boolean =
        packageName == other.packageName && process == other.process
}