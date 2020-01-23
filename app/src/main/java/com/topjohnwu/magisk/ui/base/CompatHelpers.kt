package com.topjohnwu.magisk.ui.base

import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.FilterableDiffObservableList
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter
import me.tatarka.bindingcollectionadapter2.ItemBinding
import me.tatarka.bindingcollectionadapter2.OnItemBind

inline fun <T : ComparableRvItem<*>> diffListOf(
    vararg newItems: T
) = diffListOf(newItems.toList())

inline fun <T : ComparableRvItem<*>> diffListOf(
    newItems: List<T>
) = DiffObservableList(object : DiffObservableList.Callback<T> {
    override fun areItemsTheSame(oldItem: T, newItem: T) = oldItem.genericItemSameAs(newItem)
    override fun areContentsTheSame(oldItem: T, newItem: T) = oldItem.genericContentSameAs(newItem)
}).also { it.update(newItems) }

inline fun <T : ComparableRvItem<*>> filterableListOf(
    vararg newItems: T
) = FilterableDiffObservableList(object : DiffObservableList.Callback<T> {
    override fun areItemsTheSame(oldItem: T, newItem: T) = oldItem.genericItemSameAs(newItem)
    override fun areContentsTheSame(oldItem: T, newItem: T) = oldItem.genericContentSameAs(newItem)
}).also { it.update(newItems.toList()) }

fun <T : ComparableRvItem<*>> adapterOf() = object : BindingRecyclerViewAdapter<T>() {
    override fun onBindBinding(
        binding: ViewDataBinding,
        variableId: Int,
        layoutRes: Int,
        position: Int,
        item: T
    ) {
        super.onBindBinding(binding, variableId, layoutRes, position, item)
        item.onBindingBound(binding)
    }
}

inline fun <T : ComparableRvItem<*>> itemBindingOf(
    crossinline body: (ItemBinding<*>) -> Unit = {}
) = OnItemBind<T> { itemBinding, _, item ->
    item.bind(itemBinding)
    body(itemBinding)
}
