package com.topjohnwu.magisk.databinding

import androidx.annotation.CallSuper
import androidx.databinding.PropertyChangeRegistry
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.BR
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

interface RvContainer<E> {
    val item: E
}

interface ComparableRv<T> : Comparable<T> {
    @Suppress("UNCHECKED_CAST")
    fun comparableEqual(o: Any?) =
        o != null && o::class == this::class && compareTo(o as T) == 0
}

abstract class DiffRvItem<T> : RvItem() {

    // Defer to contentSameAs by default
    open fun itemSameAs(other: T) = true

    open fun contentSameAs(other: T) =
        when (this) {
            is RvContainer<*> -> item == (other as RvContainer<*>).item
            is ComparableRv<*> -> comparableEqual(other)
            else -> this == other
        }

    companion object {
        private val callback = object : DiffObservableList.Callback<DiffRvItem<Any>> {
            override fun areItemsTheSame(
                oldItem: DiffRvItem<Any>,
                newItem: DiffRvItem<Any>
            ): Boolean {
                return oldItem::class == newItem::class && oldItem.itemSameAs(newItem)
            }

            override fun areContentsTheSame(
                oldItem: DiffRvItem<Any>,
                newItem: DiffRvItem<Any>
            ): Boolean {
                return oldItem.contentSameAs(newItem)
            }
        }

        @Suppress("UNCHECKED_CAST")
        fun <T : AnyDiffRvItem> callback() = callback as DiffObservableList.Callback<T>
    }
}

typealias AnyDiffRvItem = DiffRvItem<*>

abstract class ObservableDiffRvItem<T> : DiffRvItem<T>(), ObservableHost {
    override var callbacks: PropertyChangeRegistry? = null
}

abstract class ObservableRvItem : RvItem(), ObservableHost {
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
