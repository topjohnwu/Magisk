package com.topjohnwu.magisk.databinding

import androidx.annotation.MainThread
import androidx.databinding.ListChangeRegistry
import androidx.databinding.ObservableList
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListUpdateCallback
import java.util.AbstractList

open class DiffObservableList<T>(
    private val callback: Callback<T>
) : AbstractList<T>(), ObservableList<T> {

    protected var list: List<T> = emptyList()
    private val listeners = ListChangeRegistry()
    protected val listCallback = ObservableListUpdateCallback()

    override val size: Int get() = list.size

    override fun get(index: Int) = list[index]

    /**
     * Calculates the list of update operations that can convert this list into the given one.
     *
     * @param newItems The items that this list will be set to.
     * @return A DiffResult that contains the information about the edit sequence to covert this
     * list into the given one.
     */
    fun calculateDiff(newItems: List<T>): DiffUtil.DiffResult {
        return doCalculateDiff(list, newItems)
    }

    protected fun doCalculateDiff(oldItems: List<T>, newItems: List<T>): DiffUtil.DiffResult {
        return DiffUtil.calculateDiff(object : DiffUtil.Callback() {
            override fun getOldListSize() = oldItems.size

            override fun getNewListSize() = newItems.size

            override fun areItemsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems[newItemPosition]
                return callback.areItemsTheSame(oldItem, newItem)
            }

            override fun areContentsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems[newItemPosition]
                return callback.areContentsTheSame(oldItem, newItem)
            }
        }, true)
    }

    /**
     * Updates the contents of this list to the given one using the DiffResults to dispatch change
     * notifications.
     *
     * @param newItems   The items to set this list to.
     * @param diffResult The diff results to dispatch change notifications.
     */
    @MainThread
    fun update(newItems: List<T>, diffResult: DiffUtil.DiffResult) {
        list = ArrayList(newItems)
        diffResult.dispatchUpdatesTo(listCallback)
    }

    override fun addOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.add(listener)
    }

    override fun removeOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.remove(listener)
    }

    private fun notifyAdd(start: Int, count: Int) {
        listeners.notifyInserted(this, start, count)
    }

    private fun notifyRemove(start: Int, count: Int) {
        listeners.notifyRemoved(this, start, count)
    }

    interface Callback<T> {
        fun areItemsTheSame(oldItem: T, newItem: T): Boolean
        fun areContentsTheSame(oldItem: T, newItem: T): Boolean
    }

    inner class ObservableListUpdateCallback : ListUpdateCallback {
        override fun onChanged(position: Int, count: Int, payload: Any?) {
            listeners.notifyChanged(this@DiffObservableList, position, count)
        }

        override fun onMoved(fromPosition: Int, toPosition: Int) {
            listeners.notifyMoved(this@DiffObservableList, fromPosition, toPosition, 1)
        }

        override fun onInserted(position: Int, count: Int) {
            modCount += 1
            listeners.notifyInserted(this@DiffObservableList, position, count)
        }

        override fun onRemoved(position: Int, count: Int) {
            modCount += 1
            listeners.notifyRemoved(this@DiffObservableList, position, count)
        }
    }
}
