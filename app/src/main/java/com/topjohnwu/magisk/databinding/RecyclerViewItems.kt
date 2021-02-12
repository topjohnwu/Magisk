package com.topjohnwu.magisk.databinding

import androidx.annotation.CallSuper
import androidx.databinding.PropertyChangeRegistry
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.ObservableHost
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

    // Use Any.equals by default
    open fun itemSameAs(other: T) = this == other

    // Use compareTo if this is Comparable or assume not same
    @Suppress("UNCHECKED_CAST")
    open fun contentSameAs(other: T) =
        (this as? Comparable<T>)?.run { compareTo(other) == 0 } ?: false

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

abstract class ObservableItem<T> : ComparableRvItem<T>(), ObservableHost {
    override var callbacks: PropertyChangeRegistry? = null
}

/**
 * This item addresses issues where enclosing recycler has to be invalidated or generally
 * manipulated with. This shouldn't be however necessary for 99.9% of use-cases. Refrain from using
 * this item as it provides virtually no additional functionality. Stick with ComparableRvItem.
 * */

interface LenientRvItem {
    fun onBindingBound(binding: ViewDataBinding, recyclerView: RecyclerView)
}
