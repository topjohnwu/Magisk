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

class HideRvItem(
    val info: HideAppInfo
) : ObservableItem<HideRvItem>(), Comparable<HideRvItem> {

    override val layoutRes get() = R.layout.item_hide_md2

    val processes = info.processes.map { HideProcessRvItem(it) }

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
                processes
                    .filterNot { it.isHidden }
                    .filter { isExpanded || it.defaultSelection }
            } else {
                processes
                    .filter { it.isHidden }
            }.forEach { it.toggle() }
        }

    init {
        processes.forEach { it.addOnPropertyChangedCallback(BR.hidden) { recalculateChecked() } }
        addOnPropertyChangedCallback(BR.expanded) { recalculateChecked() }
        recalculateChecked()
    }

    fun toggleExpand(v: View) {
        (v.parent as? ViewGroup)?.startAnimations()
        isExpanded = !isExpanded
    }

    private fun recalculateChecked() {
        itemsChecked = processes.count { it.isHidden }
        state = if (isExpanded) {
            when (itemsChecked) {
                0 -> false
                processes.size -> true
                else -> null
            }
        } else {
            val defaultProcesses = processes.filter { it.defaultSelection }
            when (defaultProcesses.count { it.isHidden }) {
                0 -> false
                defaultProcesses.size -> true
                else -> null
            }
        }
    }

    override fun compareTo(other: HideRvItem) = comparator.compare(this, other)

    companion object {
        private val comparator = compareBy<HideRvItem>(
            { it.itemsChecked == 0 },
            { it.info }
        )
    }

}

class HideProcessRvItem(
    val process: HideProcessInfo
) : ObservableItem<HideProcessRvItem>() {

    override val layoutRes get() = R.layout.item_hide_process_md2

    val displayName = if (process.isIsolated) "(isolated) ${process.name}" else process.name

    @get:Bindable
    var isHidden
        get() = process.isHidden
        set(value) = set(value, process.isHidden, { process.isHidden = it }, BR.hidden) {
            val arg = if (it) "add" else "rm"
            val (name, pkg) = process
            Shell.su("magiskhide $arg $pkg \'$name\'").submit()
        }

    fun toggle() {
        isHidden = !isHidden
    }

    val defaultSelection get() =
        process.isIsolated || process.isAppZygote || process.name == process.packageName

    override fun contentSameAs(other: HideProcessRvItem) =
        process.isHidden == other.process.isHidden

    override fun itemSameAs(other: HideProcessRvItem) =
        process.name == other.process.name && process.packageName == other.process.packageName

}
