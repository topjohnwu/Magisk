package com.topjohnwu.magisk.extensions

import androidx.collection.SparseArrayCompat
import androidx.databinding.ObservableList
import com.topjohnwu.magisk.utils.DiffObservableList
import kotlinx.coroutines.*

fun <T> MutableList<T>.update(newList: List<T>) {
    clear()
    addAll(newList)
}

fun List<String>.toShellCmd(): String {
    val sb = StringBuilder()
    for (s in this) {
        if (s.contains(" ")) {
            sb.append('"').append(s).append('"')
        } else {
            sb.append(s)
        }
        sb.append(' ')
    }
    sb.deleteCharAt(sb.length - 1)
    return sb.toString()
}

fun <T1, T2> ObservableList<T1>.sendUpdatesTo(
    target: DiffObservableList<T2>,
    scope: CoroutineScope,
    mapper: (List<T1>) -> List<T2>
) = addOnListChangedCallback(object :
    ObservableList.OnListChangedCallback<ObservableList<T1>>() {
    override fun onChanged(sender: ObservableList<T1>?) {
        updateAsync(sender ?: return)
    }

    override fun onItemRangeRemoved(sender: ObservableList<T1>?, p0: Int, p1: Int) {
        updateAsync(sender ?: return)
    }

    override fun onItemRangeMoved(sender: ObservableList<T1>?, p0: Int, p1: Int, p2: Int) {
        updateAsync(sender ?: return)
    }

    override fun onItemRangeInserted(sender: ObservableList<T1>?, p0: Int, p1: Int) {
        updateAsync(sender ?: return)
    }

    override fun onItemRangeChanged(sender: ObservableList<T1>?, p0: Int, p1: Int) {
        updateAsync(sender ?: return)
    }

    private var updater: Job? = null

    private fun updateAsync(sender: List<T1>) {
        updater?.cancel()
        updater = scope.launch {
            val (list, diff) = withContext(Dispatchers.Default) {
                val list = mapper(sender)
                list to target.calculateDiff(list)
            }
            target.update(list, diff)
        }
    }
})

fun <T1> ObservableList<T1>.copyNewInputInto(
    target: MutableList<T1>
) = addOnListChangedCallback(object : ObservableList.OnListChangedCallback<ObservableList<T1>>() {
    override fun onChanged(p0: ObservableList<T1>?) = Unit
    override fun onItemRangeRemoved(p0: ObservableList<T1>?, p1: Int, p2: Int) = Unit
    override fun onItemRangeMoved(p0: ObservableList<T1>?, p1: Int, p2: Int, p3: Int) = Unit
    override fun onItemRangeChanged(p0: ObservableList<T1>?, p1: Int, p2: Int) = Unit
    override fun onItemRangeInserted(
        sender: ObservableList<T1>?,
        positionStart: Int,
        itemCount: Int
    ) {
        val positionEnd = positionStart + itemCount
        val addedValues = sender?.slice(positionStart until positionEnd).orEmpty()
        target.addAll(addedValues)
    }
})

operator fun <E> SparseArrayCompat<E>.set(key: Int, value: E) {
    put(key, value)
}
