package com.topjohnwu.magisk.databinding

import androidx.annotation.MainThread
import androidx.annotation.WorkerThread
import androidx.databinding.ListChangeRegistry
import androidx.databinding.ObservableList
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListUpdateCallback
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.AbstractList

open class DiffObservableList<T : DiffItem<*>>
    : AbstractList<T>(), ObservableList<T>, ListUpdateCallback {

    protected var list: List<T> = emptyList()
        private set
    private val listeners = ListChangeRegistry()

    override val size: Int get() = list.size

    override fun get(index: Int) = list[index]

    fun calculateDiff(newItems: List<T>): DiffUtil.DiffResult {
        return doCalculateDiff(list, newItems)
    }

    protected fun doCalculateDiff(oldItems: List<T>, newItems: List<T>): DiffUtil.DiffResult {
        return DiffUtil.calculateDiff(object : DiffUtil.Callback() {
            override fun getOldListSize() = oldItems.size

            override fun getNewListSize() = newItems.size

            @Suppress("UNCHECKED_CAST")
            override fun areItemsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems[newItemPosition]
                return (oldItem as DiffItem<Any>).itemSameAs(newItem)
            }

            @Suppress("UNCHECKED_CAST")
            override fun areContentsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems[newItemPosition]
                return (oldItem as DiffItem<Any>).contentSameAs(newItem)
            }
        }, true)
    }

    @MainThread
    fun update(newItems: List<T>, diffResult: DiffUtil.DiffResult) {
        list = ArrayList(newItems)
        diffResult.dispatchUpdatesTo(this)
    }

    @WorkerThread
    suspend fun update(newItems: List<T>) {
        val diffResult = calculateDiff(newItems)
        withContext(Dispatchers.Main) {
            update(newItems, diffResult)
        }
    }

    override fun addOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.add(listener)
    }

    override fun removeOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.remove(listener)
    }

    override fun onChanged(position: Int, count: Int, payload: Any?) {
        listeners.notifyChanged(this, position, count)
    }

    override fun onMoved(fromPosition: Int, toPosition: Int) {
        listeners.notifyMoved(this, fromPosition, toPosition, 1)
    }

    override fun onInserted(position: Int, count: Int) {
        modCount += 1
        listeners.notifyInserted(this, position, count)
    }

    override fun onRemoved(position: Int, count: Int) {
        modCount += 1
        listeners.notifyRemoved(this, position, count)
    }
}
