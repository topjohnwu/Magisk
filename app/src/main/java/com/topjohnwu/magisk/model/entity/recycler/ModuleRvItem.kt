package com.topjohnwu.magisk.model.entity.recycler

import android.content.res.Resources
import androidx.annotation.StringRes
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.OldModule
import com.topjohnwu.magisk.model.entity.Repo
import com.topjohnwu.magisk.model.entity.Repository
import com.topjohnwu.magisk.utils.get
import com.topjohnwu.magisk.utils.toggle

class ModuleRvItem(val item: OldModule) : ComparableRvItem<ModuleRvItem>() {

    override val layoutRes: Int = R.layout.item_module

    val lastActionNotice = KObservableField("")
    val isChecked = KObservableField(item.isEnabled)
    val isDeletable = KObservableField(item.willBeRemoved())

    init {
        isChecked.addOnPropertyChangedCallback {
            when (it) {
                true -> item.removeDisableFile().notice(R.string.disable_file_removed)
                false -> item.createDisableFile().notice(R.string.disable_file_created)
            }
        }
        isDeletable.addOnPropertyChangedCallback {
            when (it) {
                true -> item.createRemoveFile().notice(R.string.remove_file_created)
                false -> item.deleteRemoveFile().notice(R.string.remove_file_deleted)
            }
        }
        when {
            item.isUpdated -> notice(R.string.update_file_created)
            item.willBeRemoved() -> notice(R.string.remove_file_created)
        }
    }

    fun toggle() = isChecked.toggle()
    fun toggleDelete() = isDeletable.toggle()

    @Suppress("unused")
    private fun Any.notice(@StringRes info: Int) {
        lastActionNotice.value = get<Resources>().getString(info)
    }

    override fun contentSameAs(other: ModuleRvItem): Boolean = item.version == other.item.version
            && item.versionCode == other.item.versionCode
            && item.description == other.item.description
            && item.name == other.item.name

    override fun itemSameAs(other: ModuleRvItem): Boolean = item.id == other.item.id
}

class RepoRvItem(val item: Repo) : ComparableRvItem<RepoRvItem>() {

    constructor(repo: Repository) : this(Repo(repo))

    override val layoutRes: Int = R.layout.item_repo

    override fun contentSameAs(other: RepoRvItem): Boolean = item.version == other.item.version
            && item.lastUpdate == other.item.lastUpdate
            && item.versionCode == other.item.versionCode
            && item.description == other.item.description
            && item.detailUrl == other.item.detailUrl

    override fun itemSameAs(other: RepoRvItem): Boolean = item.id == other.item.id
}