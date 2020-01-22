package com.topjohnwu.magisk.ui.base

import android.os.Handler
import android.os.Looper

interface Queryable {

    val queryDelay: Long
    val queryHandler: Handler
    val queryRunnable: Runnable

    fun submitQuery()

    companion object {
        fun impl(delay: Long = 1000L) = object : Queryable {
            override val queryDelay = delay
            override val queryHandler = Handler(Looper.getMainLooper())
            override val queryRunnable = Runnable { TODO() }

            override fun submitQuery() {}
        }
    }
}
