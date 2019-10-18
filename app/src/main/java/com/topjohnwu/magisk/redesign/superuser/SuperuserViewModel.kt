package com.topjohnwu.magisk.redesign.superuser

import android.content.pm.PackageManager
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.data.database.PolicyDao
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.applySchedulers
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.recycler.PolicyRvItem
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.utils.DiffObservableList

class SuperuserViewModel(
    private val db: PolicyDao,
    private val packageManager: PackageManager
) : CompatViewModel() {

    val items = diffListOf<PolicyRvItem>()
    val itemBinding = itemBindingOf<PolicyRvItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = db.fetchAll()
        .flattenAsFlowable { it }
        .parallel()
        .map { PolicyRvItem(it, it.applicationInfo.loadIcon(packageManager)) }
        .sequential()
        .sorted { o1, o2 ->
            compareBy<PolicyRvItem>(
                { it.item.appName.toLowerCase() },
                { it.item.packageName }
            ).compare(o1, o2)
        }
        .toList()
        .map { it to items.calculateDiff(it) }
        .applySchedulers()
        .applyViewModel(this)
        .subscribeK { items.update(it.first, it.second) }

    fun hidePressed() = Navigation.hide().publish()

    fun deletePressed(item: PolicyRvItem) {
        TODO()
    }

}

inline fun <T : ComparableRvItem<T>> diffListOf(
    vararg newItems: T
) = DiffObservableList(object : DiffObservableList.Callback<T> {
    override fun areItemsTheSame(oldItem: T, newItem: T) = oldItem.genericItemSameAs(newItem)
    override fun areContentsTheSame(oldItem: T, newItem: T) = oldItem.genericContentSameAs(newItem)
}).also { it.update(newItems.toList()) }