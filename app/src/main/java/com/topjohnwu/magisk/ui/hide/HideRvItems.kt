package com.topjohnwu.magisk.ui.hide

import android.view.View
import android.view.ViewGroup
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.utils.addOnPropertyChangedCallback
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlin.math.roundToInt

class HideItem(
    app: HideAppTarget
) : ObservableItem<HideItem>(), Comparable<HideItem> {

    override val layoutRes = R.layout.item_hide_md2

    val info = app.info
    val processes = app.processes.map { HideProcessItem(it) }

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    var itemsChecked = 0
        set(value) = set(value, field, { field = it }, BR.checkedPercent)

    @get:Bindable
    val checkedPercent get() = (itemsChecked.toFloat() / processes.size * 100).roundToInt()

    private var state: Boolean? = false
        set(value) = set(value, field, { field = it }, BR.hiddenState)

    @get:Bindable
    var hiddenState: Boolean?
        get() = state
        set(value) = set(value, state, { state = it }, BR.hiddenState) {
            if (value == true) {
                processes.filterNot { it.isHidden }
            } else {
                processes
            }.forEach { it.toggle() }
        }

    init {
        processes.forEach { it.addOnPropertyChangedCallback(BR.hidden) { recalculateChecked() } }
        recalculateChecked()
    }

    fun toggleExpand(v: View) {
        (v.parent as? ViewGroup)?.startAnimations()
        isExpanded = !isExpanded
    }

    private fun recalculateChecked() {
        itemsChecked = processes.count { it.isHidden }
        state = when (itemsChecked) {
            0 -> false
            processes.size -> true
            else -> null
        }
    }

    override fun compareTo(other: HideItem) = comparator.compare(this, other)

    companion object {
        private val comparator = compareBy<HideItem>(
            { it.itemsChecked == 0 },
            { it.info }
        )
    }

}

class HideProcessItem(
    val process: HideProcessInfo
) : ObservableItem<HideProcessItem>() {

    override val layoutRes = R.layout.item_hide_process_md2

    @get:Bindable
    var isHidden = process.isHidden
        set(value) = set(value, field, { field = it }, BR.hidden) {
            val arg = if (isHidden) "add" else "rm"
            val (name, pkg) = process
            Shell.su("magiskhide --$arg $pkg $name").submit()
        }

    fun toggle() {
        isHidden = !isHidden
    }

    override fun contentSameAs(other: HideProcessItem) = process == other.process
    override fun itemSameAs(other: HideProcessItem) = process.name == other.process.name

}
