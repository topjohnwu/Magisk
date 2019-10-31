package com.topjohnwu.magisk.utils

import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import java.util.*

class FilterableDiffObservableList<T>(
    callback: Callback<T>
) : DiffObservableList<T>(callback) {

    var filter: ((T) -> Boolean)? = null
        set(value) {
            field = value
            queueUpdate()
        }
    @Volatile
    private var sublist: MutableList<T> = super.list

    // ---

    private val ui by lazy { Handler(Looper.getMainLooper()) }
    private val handler = Handler(HandlerThread("List${hashCode()}").apply { start() }.looper)
    private val updater = Runnable {
        val filter = filter ?: { true }
        val newList = super.list.filter(filter)
        val diff = synchronized(this) { doCalculateDiff(sublist, newList) }
        ui.post {
            sublist = Collections.synchronizedList(newList)
            diff.dispatchUpdatesTo(listCallback)
        }
    }

    private fun queueUpdate() {
        handler.removeCallbacks(updater)
        handler.post(updater)
    }

    fun hasFilter() = filter != null

    fun filter(switch: (T) -> Boolean) {
        filter = switch
    }

    fun reset() {
        filter = null
    }

    // ---

    override fun get(index: Int): T {
        return sublist.get(index)
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