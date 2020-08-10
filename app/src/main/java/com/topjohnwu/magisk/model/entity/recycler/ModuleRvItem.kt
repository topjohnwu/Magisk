package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.Bindable
import androidx.databinding.Observable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.module.Module
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.utils.set

object InstallModule : ComparableRvItem<InstallModule>() {
    override val layoutRes = R.layout.item_module_download

    override fun contentSameAs(other: InstallModule) = this == other
    override fun itemSameAs(other: InstallModule) = this === other
}

class SectionTitle(
    val title: Int,
    _button: Int = 0,
    _icon: Int = 0
) : ObservableItem<SectionTitle>() {
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

    override fun itemSameAs(other: SectionTitle): Boolean = this === other
    override fun contentSameAs(other: SectionTitle): Boolean = this === other
}

sealed class RepoItem(val item: Repo) : ObservableItem<RepoItem>() {
    override val layoutRes: Int = R.layout.item_repo_md2

    @get:Bindable
    var progress = 0
        set(value) = set(value, field, { field = it }, BR.progress)

    @get:Bindable
    var isUpdate = false
        set(value) = set(value, field, { field = it }, BR.update)


    override fun contentSameAs(other: RepoItem): Boolean = item == other.item
    override fun itemSameAs(other: RepoItem): Boolean = item.id == other.item.id

    class Update(item: Repo) : RepoItem(item) {
        init {
            isUpdate = true
        }
    }

    class Remote(item: Repo) : RepoItem(item)
}

class ModuleItem(val item: Module) : ObservableItem<ModuleItem>(), Observable {

    override val layoutRes = R.layout.item_module_md2

    @get:Bindable
    var repo: Repo? = null
        set(value) = set(value, field, { field = it }, BR.repo)

    @get:Bindable
    var isEnabled
        get() = item.enable
        set(value) {
            item.enable = value
            notifyPropertyChanged(BR.enabled)
        }

    @get:Bindable
    var isRemoved
        get() = item.remove
        set(value) {
            item.remove = value
            notifyPropertyChanged(BR.removed)
        }

    val isUpdated get() = item.updated
    val isModified get() = isRemoved || isUpdated

    fun toggle() {
        isEnabled = !isEnabled
    }

    fun delete(viewModel: ModuleViewModel) {
        isRemoved = !isRemoved
        viewModel.updateActiveState()
    }

    override fun contentSameAs(other: ModuleItem): Boolean = item.version == other.item.version
            && item.versionCode == other.item.versionCode
            && item.description == other.item.description
            && item.name == other.item.name

    override fun itemSameAs(other: ModuleItem): Boolean = item.id == other.item.id
}

