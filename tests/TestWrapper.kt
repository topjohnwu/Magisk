package com.topjohnwu.magisk.tests

import com.topjohnwu.magisk.core.logging.JSONLogger
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking

object TestWrapper {
    /**
     * Run an action with retries and structured logging.
     * action: suspend lambda to run.
     */
    fun runWithRetries(maxAttempts: Int = 3, delayMs: Long = 2000, actionName: String = "testAction", action: suspend () -> Boolean) {
        runBlocking {
            var attempt = 1
            while (attempt <= maxAttempts) {
                JSONLogger.info("TestWrapper", "attempt_start", null, mapOf("attempt" to attempt, "action" to actionName))
                val ok = try {
                    action()
                } catch (t: Throwable) {
                    JSONLogger.error("TestWrapper", "exception", null, mapOf("attempt" to attempt, "error" to t.toString()))
                    false
                }
                if (ok) {
                    JSONLogger.info("TestWrapper", "attempt_success", null, mapOf("attempt" to attempt, "action" to actionName))
                    return@runBlocking
                }
                JSONLogger.info("TestWrapper", "attempt_failed", null, mapOf("attempt" to attempt, "action" to actionName))
                attempt++
                delay(delayMs)
            }
            JSONLogger.error("TestWrapper", "all_attempts_failed", null, mapOf("action" to actionName))
        }
    }
}
