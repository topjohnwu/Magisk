package com.topjohnwu.magisk.extensions

import androidx.databinding.ObservableList
import com.topjohnwu.magisk.utils.KObservableField


fun KObservableField<Boolean>.toggle() {
    value = !value
}

fun <T> ObservableList<T>.addOnListChangedCallback(
    onChanged: ((sender: ObservableList<T>) -> Unit)? = null,
    onItemRangeRemoved: ((sender: ObservableList<T>, positionStart: Int, itemCount: Int) -> Unit)? = null,
    onItemRangeMoved: ((sender: ObservableList<T>, fromPosition: Int, toPosition: Int, itemCount: Int) -> Unit)? = null,
    onItemRangeInserted: ((sender: ObservableList<T>, positionStart: Int, itemCount: Int) -> Unit)? = null,
    onItemRangeChanged: ((sender: ObservableList<T>, positionStart: Int, itemCount: Int) -> Unit)? = null
) = addOnListChangedCallback(object : ObservableList.OnListChangedCallback<ObservableList<T>>() {
    override fun onChanged(sender: ObservableList<T>?) {
        onChanged?.invoke(sender ?: return)
    }

    override fun onItemRangeRemoved(
        sender: ObservableList<T>?,
        positionStart: Int,
        itemCount: Int
    ) {
        onItemRangeRemoved?.invoke(
            sender ?: return,
            positionStart,
            itemCount
        )
    }

    override fun onItemRangeMoved(
        sender: ObservableList<T>?,
        fromPosition: Int,
        toPosition: Int,
        itemCount: Int
    ) {
        onItemRangeMoved?.invoke(
            sender ?: return,
            fromPosition,
            toPosition,
            itemCount
        )
    }

    override fun onItemRangeInserted(
        sender: ObservableList<T>?,
        positionStart: Int,
        itemCount: Int
    ) {
        onItemRangeInserted?.invoke(
            sender ?: return,
            positionStart,
            itemCount
        )
    }

    override fun onItemRangeChanged(
        sender: ObservableList<T>?,
        positionStart: Int,
        itemCount: Int
    ) {
        onItemRangeChanged?.invoke(
            sender ?: return,
            positionStart,
            itemCount
        )
    }
})