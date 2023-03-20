package com.topjohnwu.magisk.databinding

import androidx.databinding.PropertyChangeRegistry
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import kotlinx.coroutines.CoroutineScope

abstract class RvItem {
    abstract val layoutRes: Int
}

interface RvContainer<E> {
    val item: E
}

interface ViewAwareRvItem {
    fun onBind(binding: ViewDataBinding, recyclerView: RecyclerView)
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
}

typealias AnyDiffRvItem = DiffRvItem<*>

abstract class ObservableDiffRvItem<T> : DiffRvItem<T>(), ObservableHost {
    override var callbacks: PropertyChangeRegistry? = null
}

abstract class ObservableRvItem : RvItem(), ObservableHost {
    override var callbacks: PropertyChangeRegistry? = null
}

private object DiffRvItemCallback : DiffObservableList.Callback<DiffRvItem<Any>> {
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
class DiffRvItemList<T: AnyDiffRvItem> : DiffObservableList<T>(DiffRvItemCallback as Callback<T>)

@Suppress("UNCHECKED_CAST")
class DiffRvItemFilterList<T: AnyDiffRvItem>(
    scope: CoroutineScope
) : FilterableDiffObservableList<T>(DiffRvItemCallback as Callback<T>, scope)
