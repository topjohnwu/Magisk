package com.topjohnwu.magisk.databinding

import androidx.databinding.PropertyChangeRegistry
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView

abstract class RvItem {
    abstract val layoutRes: Int
}

abstract class ObservableRvItem : RvItem(), ObservableHost {
    override var callbacks: PropertyChangeRegistry? = null
}

interface ItemWrapper<E> {
    val item: E
}

interface ViewAwareItem {
    fun onBind(binding: ViewDataBinding, recyclerView: RecyclerView)
}

interface DiffItem<T : Any> {

    fun itemSameAs(other: T): Boolean {
        if (this === other) return true
        return when (this) {
            is ItemWrapper<*> -> item == (other as ItemWrapper<*>).item
            is Comparable<*> -> compareValues(this, other as Comparable<*>) == 0
            else -> this == other
        }
    }

    fun contentSameAs(other: T) = true
}
