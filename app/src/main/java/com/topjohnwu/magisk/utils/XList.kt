package com.topjohnwu.magisk.utils

import androidx.databinding.ObservableList
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.DiffObservableList
import io.reactivex.disposables.Disposable

fun <T> MutableList<T>.update(newList: List<T>) {
    clear()
    addAll(newList)
}

fun <T1, T2> ObservableList<T1>.sendUpdatesTo(
    target: DiffObservableList<T2>,
    mapper: (List<T1>) -> List<T2>
) {
    addOnListChangedCallback(object :
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

        private var updater: Disposable? = null

        private fun updateAsync(sender: List<T1>) {
            updater?.dispose()
            updater = sender.toSingle()
                .map { mapper(it) }
                .map { it to target.calculateDiff(it) }
                .subscribeK { target.update(it.first, it.second) }
        }
    })
}