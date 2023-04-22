package com.topjohnwu.magisk.databinding

import androidx.annotation.MainThread
import androidx.annotation.WorkerThread
import androidx.databinding.ListChangeRegistry
import androidx.databinding.ObservableList
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListUpdateCallback
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.AbstractList

// Only expose the immutable List types
interface DiffList<T : DiffItem<*>> : List<T> {
    fun calculateDiff(newItems: List<T>): DiffUtil.DiffResult

    @MainThread
    fun update(newItems: List<T>, diffResult: DiffUtil.DiffResult)

    @WorkerThread
    suspend fun update(newItems: List<T>)
}

interface FilterList<T : DiffItem<*>> : List<T> {
    fun filter(filter: (T) -> Boolean)

    @MainThread
    fun set(newItems: List<T>)
}

fun <T : DiffItem<*>> diffList(): DiffList<T> = DiffObservableList()

fun <T : DiffItem<*>> filterList(scope: CoroutineScope): FilterList<T> =
    FilterableDiffObservableList(scope)

private open class DiffObservableList<T : DiffItem<*>>
    : AbstractList<T>(), ObservableList<T>, DiffList<T>, ListUpdateCallback {

    protected var list: List<T> = emptyList()
    private val listeners = ListChangeRegistry()

    override val size: Int get() = list.size

    override fun get(index: Int) = list[index]

    override fun calculateDiff(newItems: List<T>): DiffUtil.DiffResult {
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
    override fun update(newItems: List<T>, diffResult: DiffUtil.DiffResult) {
        list = ArrayList(newItems)
        diffResult.dispatchUpdatesTo(this)
    }

    @WorkerThread
    override suspend fun update(newItems: List<T>) {
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

private class FilterableDiffObservableList<T : DiffItem<*>>(
    private val scope: CoroutineScope
) : DiffObservableList<T>(), FilterList<T> {

    private var sublist: List<T> = emptyList()
    private var job: Job? = null
    private var lastFilter: ((T) -> Boolean)? = null

    // ---

    override fun filter(filter: (T) -> Boolean) {
        lastFilter = filter
        job?.cancel()
        job = scope.launch(Dispatchers.Default) {
            val oldList = sublist
            val newList = list.filter(filter)
            val diff = doCalculateDiff(oldList, newList)
            withContext(Dispatchers.Main) {
                sublist = newList
                diff.dispatchUpdatesTo(this@FilterableDiffObservableList)
            }
        }
    }

    // ---

    override fun get(index: Int): T {
        return sublist[index]
    }

    override val size: Int
        get() = sublist.size

    @MainThread
    override fun set(newItems: List<T>) {
        onRemoved(0, sublist.size)
        list = newItems
        sublist = emptyList()
        lastFilter?.let { filter(it) }
    }
}
