package com.topjohnwu.magisk.core.base

import android.content.Context
import android.net.Network
import android.net.Uri
import androidx.annotation.MainThread
import androidx.annotation.RequiresApi
import androidx.work.Data
import androidx.work.ListenableWorker
import com.google.common.util.concurrent.ListenableFuture
import java.util.*

abstract class BaseWorkerWrapper {

    private lateinit var worker: ListenableWorker

    val applicationContext: Context
        get() = worker.applicationContext

    val id: UUID
        get() = worker.id

    val inputData: Data
        get() = worker.inputData

    val tags: Set<String>
        get() = worker.tags

    val triggeredContentUris: List<Uri>
        @RequiresApi(24)
        get() = worker.triggeredContentUris

    val triggeredContentAuthorities: List<String>
        @RequiresApi(24)
        get() = worker.triggeredContentAuthorities

    val network: Network?
        @RequiresApi(28)
        get() = worker.network

    val runAttemptCount: Int
        get() = worker.runAttemptCount

    val isStopped: Boolean
        get() = worker.isStopped

    abstract fun doWork(): ListenableWorker.Result

    fun onStopped() {}

    fun attachWorker(w: ListenableWorker) {
        worker = w
    }

    @MainThread
    fun startWork(): ListenableFuture<ListenableWorker.Result> {
        return worker.startWork()
    }
}
