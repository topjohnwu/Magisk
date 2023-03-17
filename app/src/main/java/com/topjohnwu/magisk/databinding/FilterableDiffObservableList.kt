package com.topjohnwu.magisk.databinding

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

open class FilterableDiffObservableList<T>(
    callback: Callback<T>,
    private val scope: CoroutineScope
) : DiffObservableList<T>(callback) {

    private var sublist: List<T> = emptyList()
    private var job: Job? = null

    // ---

    fun filter(filter: (T) -> Boolean) {
        job?.cancel()
        job = scope.launch(Dispatchers.Default) {
            val oldList = sublist
            val newList = list.filter(filter)
            val diff = doCalculateDiff(oldList, newList)
            withContext(Dispatchers.Main) {
                sublist = newList
                diff.dispatchUpdatesTo(listCallback)
            }
        }
    }

    // ---

    override fun get(index: Int): T {
        return sublist[index]
    }

    override val size: Int
        get() = sublist.size
}
