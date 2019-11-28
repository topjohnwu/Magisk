package com.topjohnwu.magisk.databinding

import androidx.annotation.CallSuper
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.utils.DiffObservableList
import me.tatarka.bindingcollectionadapter2.ItemBinding

abstract class RvItem {

    abstract val layoutRes: Int

    @CallSuper
    open fun bind(binding: ItemBinding<*>) {
        binding.set(BR.item, layoutRes)
    }

    /**
     * This callback is useful if you want to manipulate your views directly.
     * If you want to use this callback, you must set [me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter]
     * on your RecyclerView and call it from there. You can use [BindingBoundAdapter] for your convenience.
     */
    open fun onBindingBound(binding: ViewDataBinding) {}
}

abstract class ComparableRvItem<in T> : RvItem() {

    abstract fun itemSameAs(other: T): Boolean
    abstract fun contentSameAs(other: T): Boolean
    @Suppress("UNCHECKED_CAST")
    open fun genericItemSameAs(other: Any): Boolean = other::class == this::class && itemSameAs(other as T)
    @Suppress("UNCHECKED_CAST")
    open fun genericContentSameAs(other: Any): Boolean = other::class == this::class && contentSameAs(other as T)

    companion object {
        val callback = object : DiffObservableList.Callback<ComparableRvItem<*>> {
            override fun areItemsTheSame(
                oldItem: ComparableRvItem<*>,
                newItem: ComparableRvItem<*>
            ) = oldItem.genericItemSameAs(newItem)

            override fun areContentsTheSame(
                oldItem: ComparableRvItem<*>,
                newItem: ComparableRvItem<*>
            ) = oldItem.genericContentSameAs(newItem)
        }
    }
}