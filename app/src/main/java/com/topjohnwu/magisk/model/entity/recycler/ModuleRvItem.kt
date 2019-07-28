package com.topjohnwu.magisk.model.entity.recycler

import android.content.res.Resources
import androidx.annotation.StringRes
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.module.Module
import com.topjohnwu.magisk.model.entity.module.Repo

class ModuleRvItem(val item: Module) : ComparableRvItem<ModuleRvItem>() {

    override val layoutRes: Int = R.layout.item_module

    val lastActionNotice = KObservableField("")
    val isChecked = KObservableField(item.enable)
    val isDeletable = KObservableField(item.remove)

    init {
        isChecked.addOnPropertyChangedCallback {
            when (it) {
                true -> {
                    item.enable = true
                    notice(R.string.disable_file_removed)
                }
                false -> {
                    item.enable = false
                    notice(R.string.disable_file_created)
                }
            }
        }
        isDeletable.addOnPropertyChangedCallback {
            when (it) {
                true -> {
                    item.remove = true
                    notice(R.string.remove_file_created)
                }
                false -> {
                    item.remove = false
                    notice(R.string.remove_file_deleted)
                }
            }
        }
        when {
            item.updated -> notice(R.string.update_file_created)
            item.remove -> notice(R.string.remove_file_created)
        }
    }

    fun toggle() = isChecked.toggle()
    fun toggleDelete() = isDeletable.toggle()

    private fun notice(@StringRes info: Int) {
        lastActionNotice.value = get<Resources>().getString(info)
    }

    override fun contentSameAs(other: ModuleRvItem): Boolean = item.version == other.item.version
            && item.versionCode == other.item.versionCode
            && item.description == other.item.description
            && item.name == other.item.name

    override fun itemSameAs(other: ModuleRvItem): Boolean = item.id == other.item.id
}

class RepoRvItem(val item: Repo) : ComparableRvItem<RepoRvItem>() {

    override val layoutRes: Int = R.layout.item_repo

    override fun contentSameAs(other: RepoRvItem): Boolean = item == other.item

    override fun itemSameAs(other: RepoRvItem): Boolean = item.id == other.item.id
}