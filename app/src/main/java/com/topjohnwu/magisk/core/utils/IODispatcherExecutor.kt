package com.topjohnwu.magisk.core.utils

import kotlinx.coroutines.*
import java.util.concurrent.*

class IODispatcherExecutor : AbstractExecutorService() {

    private val job = SupervisorJob().apply { invokeOnCompletion { future.run() } }
    private val scope = CoroutineScope(job + Dispatchers.IO)
    private val future = FutureTask(Callable { true })

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

    override fun awaitTermination(timeout: Long, unit: TimeUnit): Boolean {
        return try {
            future.get(timeout, unit)
        } catch (e: TimeoutException) {
            false
        }
    }
}
