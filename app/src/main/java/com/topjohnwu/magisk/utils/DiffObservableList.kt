package com.topjohnwu.magisk.utils

import androidx.annotation.MainThread
import androidx.databinding.ListChangeRegistry
import androidx.databinding.ObservableList
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListUpdateCallback
import java.util.*
import kotlin.collections.ArrayList

/**
 * @param callback    The callback that controls the behavior of the DiffObservableList.
 * @param detectMoves True if DiffUtil should try to detect moved items, false otherwise.
 */
open class DiffObservableList<T>(
        private val callback: Callback<T>,
        private val detectMoves: Boolean = true
) : AbstractList<T>(), ObservableList<T> {

    private val LIST_LOCK = Object()
    protected var list: MutableList<T> = ArrayList()
    private val listeners = ListChangeRegistry()
    protected val listCallback = ObservableListUpdateCallback()

    override val size: Int get() = list.size

    /**
     * Calculates the list of update operations that can convert this list into the given one.
     *
     * @param newItems The items that this list will be set to.
     * @return A DiffResult that contains the information about the edit sequence to covert this
     * list into the given one.
     */
    fun calculateDiff(newItems: List<T>): DiffUtil.DiffResult {
        val frozenList = synchronized(LIST_LOCK) {
            ArrayList(list)
        }
        return doCalculateDiff(frozenList, newItems)
    }

    protected fun doCalculateDiff(oldItems: List<T>, newItems: List<T>?): DiffUtil.DiffResult {
        return DiffUtil.calculateDiff(object : DiffUtil.Callback() {
            override fun getOldListSize() = oldItems.size

            override fun getNewListSize() = newItems?.size ?: 0

            override fun areItemsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems!![newItemPosition]
                return callback.areItemsTheSame(oldItem, newItem)
            }

            override fun areContentsTheSame(oldItemPosition: Int, newItemPosition: Int): Boolean {
                val oldItem = oldItems[oldItemPosition]
                val newItem = newItems!![newItemPosition]
                return callback.areContentsTheSame(oldItem, newItem)
            }
        }, detectMoves)
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
        synchronized(LIST_LOCK) {
            list = newItems.toMutableList()
        }
        diffResult.dispatchUpdatesTo(listCallback)
    }

    /**
     * Sets this list to the given items. This is a convenience method for calling [ ][.calculateDiff] followed by [.update].
     *
     *
     * **Warning!** If the lists are large this operation may be too slow for the main thread. In
     * that case, you should call [.calculateDiff] on a background thread and then
     * [.update] on the main thread.
     *
     * @param newItems The items to set this list to.
     */
    @MainThread
    fun update(newItems: List<T>) {
        val diffResult = doCalculateDiff(list, newItems)
        list = newItems.toMutableList()
        diffResult.dispatchUpdatesTo(listCallback)
    }

    override fun addOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.add(listener)
    }

    override fun removeOnListChangedCallback(listener: ObservableList.OnListChangedCallback<out ObservableList<T>>) {
        listeners.remove(listener)
    }

    override fun get(index: Int): T {
        return list[index]
    }

    override fun add(element: T): Boolean {
        list.add(element)
        notifyAdd(size - 1, 1)
        return true
    }

    override fun add(index: Int, element: T) {
        list.add(index, element)
        notifyAdd(index, 1)
    }

    override fun addAll(elements: Collection<T>): Boolean {
        val oldSize = size
        val added = list.addAll(elements)
        if (added) {
            notifyAdd(oldSize, size - oldSize)
        }
        return added
    }

    override fun addAll(index: Int, elements: Collection<T>): Boolean {
        val added = list.addAll(index, elements)
        if (added) {
            notifyAdd(index, elements.size)
        }
        return added
    }

    override fun clear() {
        val oldSize = size
        list.clear()
        if (oldSize != 0) {
            notifyRemove(0, oldSize)
        }
    }

    override fun remove(element: T): Boolean {
        val index = indexOf(element)
        return if (index >= 0) {
            removeAt(index)
            true
        } else {
            false
        }
    }

    override fun removeAt(index: Int): T {
        val element = list.removeAt(index)
        notifyRemove(index, 1)
        return element
    }

    fun removeLast(): T? {
        if (size > 0) {
            val index = size - 1
            return removeAt(index)
        }
        return null
    }

    override fun set(index: Int, element: T): T {
        val old = list.set(index, element)
        listeners.notifyChanged(this, index, 1)
        return old
    }

    private fun notifyAdd(start: Int, count: Int) {
        listeners.notifyInserted(this, start, count)
    }

    private fun notifyRemove(start: Int, count: Int) {
        listeners.notifyRemoved(this, start, count)
    }

    /**
     * A Callback class used by DiffUtil while calculating the diff between two lists.
     */
    interface Callback<T> {
        /**
         * Called by the DiffUtil to decide whether two object represent the same Item.
         *
         *
         * For example, if your items have unique ids, this method should check their id equality.
         *
         * @param oldItem The old item.
         * @param newItem The new item.
         * @return True if the two items represent the same object or false if they are different.
         */
        fun areItemsTheSame(oldItem: T, newItem: T): Boolean

        /**
         * Called by the DiffUtil when it wants to check whether two items have the same data.
         * DiffUtil uses this information to detect if the contents of an item has changed.
         *
         *
         * DiffUtil uses this method to check equality instead of [Object.equals] so
         * that you can change its behavior depending on your UI.
         *
         *
         * This method is called only if [.areItemsTheSame] returns `true` for
         * these items.
         *
         * @param oldItem The old item.
         * @param newItem The new item which replaces the old item.
         * @return True if the contents of the items are the same or false if they are different.
         */
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