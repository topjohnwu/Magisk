package com.topjohnwu.magisk.databinding

import com.topjohnwu.superuser.internal.UiThreadHandler
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.Collections

class FilterableDiffObservableList<T>(
    callback: Callback<T>,
    private val scope: CoroutineScope
) : DiffObservableList<T>(callback) {

    private var sublist: MutableList<T> = list
    private var job: Job? = null

    // ---

    fun filter(filter: (T) -> Boolean) {
        job?.cancel()
        job = scope.launch(Dispatchers.Default) {
            val newList = list.filter(filter)
            val diff = synchronized(this) { doCalculateDiff(ArrayList(sublist), newList) }
            UiThreadHandler.handler.post {
                sublist = Collections.synchronizedList(newList)
                diff.dispatchUpdatesTo(listCallback)
            }
        }
    }

    // ---

    override fun get(index: Int): T {
        return sublist[index]
    }

    override fun add(element: T): Boolean {
        return sublist.add(element)
    }

    override fun add(index: Int, element: T) {
        sublist.add(index, element)
    }

    override fun addAll(elements: Collection<T>): Boolean {
        return sublist.addAll(elements)
    }

    override fun addAll(index: Int, elements: Collection<T>): Boolean {
        return sublist.addAll(index, elements)
    }

    override fun remove(element: T): Boolean {
        return sublist.remove(element)
    }

    override fun removeAt(index: Int): T {
        return sublist.removeAt(index)
    }

    override fun set(index: Int, element: T): T {
        return sublist.set(index, element)
    }

    override val size: Int
        get() = sublist.size
}
