package com.topjohnwu.magisk.model.entity.recycler

import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.rxbus.RxBus
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.state.IndeterminateState
import com.topjohnwu.magisk.model.events.HideProcessEvent

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