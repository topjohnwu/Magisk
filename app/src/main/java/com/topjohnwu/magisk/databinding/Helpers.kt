package com.topjohnwu.magisk.databinding

import androidx.databinding.ViewDataBinding
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter
import me.tatarka.bindingcollectionadapter2.ItemBinding
import me.tatarka.bindingcollectionadapter2.OnItemBind

fun <T : AnyDiffRvItem> diffListOf() =
    DiffObservableList(DiffRvItem.callback<T>())

fun <T : AnyDiffRvItem> diffListOf(newItems: List<T>) =
    DiffObservableList(DiffRvItem.callback<T>()).also { it.update(newItems) }

fun <T : AnyDiffRvItem> filterableListOf() =
    FilterableDiffObservableList(DiffRvItem.callback<T>())

fun <T : RvItem> adapterOf() = object : BindingRecyclerViewAdapter<T>() {
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

inline fun <T : RvItem> itemBindingOf(
    crossinline body: (ItemBinding<*>) -> Unit = {}
) = OnItemBind<T> { itemBinding, _, item ->
    item.bind(itemBinding)
    body(itemBinding)
}
