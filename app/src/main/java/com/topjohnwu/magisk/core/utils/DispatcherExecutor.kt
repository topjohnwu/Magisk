package com.topjohnwu.magisk.core.utils

import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import java.util.concurrent.AbstractExecutorService
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

class DispatcherExecutor(dispatcher: CoroutineDispatcher) : AbstractExecutorService() {

    private val job = SupervisorJob()
    private val scope = CoroutineScope(job + dispatcher)
    private val latch = CountDownLatch(1)

    init {
        job.invokeOnCompletion { latch.countDown() }
    }

    override fun execute(command: Runnable) {
        scope.launch {
            command.run()
        }
    }

    override fun shutdown() = job.cancel()

    override fun shutdownNow(): List<Runnable> {
        job.cancel()
        return emptyList()
    }

    override fun isShutdown() = job.isCancelled

    override fun isTerminated() = job.isCancelled && job.isCompleted

    override fun awaitTermination(timeout: Long, unit: TimeUnit) = latch.await(timeout, unit)
}
