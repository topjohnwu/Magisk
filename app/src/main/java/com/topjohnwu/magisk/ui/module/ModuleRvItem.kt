package com.topjohnwu.magisk.ui.module

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.databinding.DiffRvItem
import com.topjohnwu.magisk.databinding.ObservableDiffRvItem
import com.topjohnwu.magisk.databinding.RvContainer
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.utils.asText

object InstallModule : DiffRvItem<InstallModule>() {
    override val layoutRes = R.layout.item_module_download
}

class SectionTitle(
    val title: Int,
    _button: Int = 0,
    _icon: Int = 0
) : ObservableDiffRvItem<SectionTitle>() {
    override val layoutRes = R.layout.item_section_md2

    @get:Bindable
    var button = _button
        set(value) = set(value, field, { field = it }, BR.button)

    @get:Bindable
    var icon = _icon
        set(value) = set(value, field, { field = it }, BR.icon)

    @get:Bindable
    var hasButton = _button != 0 && _icon != 0
        set(value) = set(value, field, { field = it }, BR.hasButton)
}

class OnlineModuleRvItem(
    override val item: OnlineModule
) : ObservableDiffRvItem<OnlineModuleRvItem>(), RvContainer<OnlineModule> {
    override val layoutRes: Int = R.layout.item_repo_md2

    @get:Bindable
    var progress = 0
        set(value) = set(value, field, { field = it }, BR.progress)

    var hasUpdate = false

    override fun itemSameAs(other: OnlineModuleRvItem): Boolean = item.id == other.item.id
}

class LocalModuleRvItem(
    override val item: LocalModule
) : ObservableDiffRvItem<LocalModuleRvItem>(), RvContainer<LocalModule> {

    override val layoutRes = R.layout.item_module_md2

    @get:Bindable
    var online: OnlineModule? = null
        set(value) = set(value, field, { field = it }, BR.online)

    @get:Bindable
    var isEnabled = item.enable
        set(value) = set(value, field, { field = it }, BR.enabled) {
            item.enable = value
        }

    @get:Bindable
    var isRemoved = item.remove
        set(value) = set(value, field, { field = it }, BR.removed) {
            item.remove = value
        }

    val isSuspended =
        (Info.isZygiskEnabled && item.isRiru) || (!Info.isZygiskEnabled && item.isZygisk)

    val suspendText =
        if (item.isRiru) R.string.suspend_text_riru.asText(R.string.zygisk.asText())
        else R.string.suspend_text_zygisk.asText(R.string.zygisk.asText())

    val isUpdated get() = item.updated
    val isModified get() = isRemoved || isUpdated

    fun delete(viewModel: ModuleViewModel) {
        isRemoved = !isRemoved
        viewModel.updateActiveState()
    }

    override fun itemSameAs(other: LocalModuleRvItem): Boolean = item.id == other.item.id
}
