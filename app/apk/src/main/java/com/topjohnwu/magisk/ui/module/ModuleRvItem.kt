package com.topjohnwu.magisk.ui.module

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.databinding.DiffItem
import com.topjohnwu.magisk.databinding.ItemWrapper
import com.topjohnwu.magisk.databinding.ObservableRvItem
import com.topjohnwu.magisk.databinding.RvItem
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.utils.TextHolder
import com.topjohnwu.magisk.utils.asText
import com.topjohnwu.magisk.core.R as CoreR

object InstallModule : RvItem(), DiffItem<InstallModule> {
    override val layoutRes = R.layout.item_module_download
}

class LocalModuleRvItem(
    override val item: LocalModule
) : ObservableRvItem(), DiffItem<LocalModuleRvItem>, ItemWrapper<LocalModule> {

    override val layoutRes = R.layout.item_module_md2

    val showNotice: Boolean
    val showAction: Boolean
    val noticeText: TextHolder

    init {
        val isZygisk = item.isZygisk
        val isRiru = item.isRiru
        val zygiskUnloaded = isZygisk && item.zygiskUnloaded

        showNotice = zygiskUnloaded ||
            (Info.isZygiskEnabled && isRiru) ||
            (!Info.isZygiskEnabled && isZygisk)
        showAction = item.hasAction && !showNotice
        noticeText =
            when {
                zygiskUnloaded -> CoreR.string.zygisk_module_unloaded.asText()
                isRiru -> CoreR.string.suspend_text_riru.asText(CoreR.string.zygisk.asText())
                else -> CoreR.string.suspend_text_zygisk.asText(CoreR.string.zygisk.asText())
            }
    }

    @get:Bindable
    var isEnabled = item.enable
        set(value) = set(value, field, { field = it }, BR.enabled, BR.updateReady) {
            item.enable = value
        }

    @get:Bindable
    var isRemoved = item.remove
        set(value) = set(value, field, { field = it }, BR.removed, BR.updateReady) {
            item.remove = value
        }

    @get:Bindable
    val showUpdate get() = item.updateInfo != null

    @get:Bindable
    val updateReady get() = item.outdated && !isRemoved && isEnabled

    val isUpdated = item.updated

    fun fetchedUpdateInfo() {
        notifyPropertyChanged(BR.showUpdate)
        notifyPropertyChanged(BR.updateReady)
    }

    fun delete() {
        isRemoved = !isRemoved
    }

    override fun itemSameAs(other: LocalModuleRvItem): Boolean = item.id == other.item.id
}
