package com.topjohnwu.magisk.core.su

import android.os.Bundle
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.runBlocking
import timber.log.Timber

object TestHandler {

    object LogList : CallbackList<String>(Runnable::run) {
        override fun onAddElement(e: String) {
            Timber.i(e)
        }
    }

    fun run(method: String): Bundle {
        var reason: String? = null

        fun prerequisite(): Boolean {
            // Make sure the Magisk app can get root
            val shell = Shell.getShell()
            if (!shell.isRoot) {
                reason = "shell not root"
                return false
            }

            // Make sure the root service is running
            RootUtils.Connection.await()
            return true
        }

        fun setup(): Boolean {
            return runBlocking {
                MagiskInstaller.Emulator(LogList, LogList).exec()
            }
        }

        fun test(): Boolean {
            // Make sure Zygisk works correctly
            if (!Info.isZygiskEnabled) {
                reason = "zygisk not enabled"
                return false
            }

            // Clear existing grant for ADB shell
            runBlocking {
                ServiceLocator.policyDB.delete(2000)
                Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
                Config.prefs.edit().commit()
            }
            return true
        }

        val result = prerequisite() && runCatching {
            when (method) {
                "setup" -> setup()
                "test" -> test()
                else -> {
                    reason = "unknown method"
                    false
                }
            }
        }.getOrElse {
            reason = it.stackTraceToString()
            false
        }

        return Bundle().apply {
            putBoolean("result", result)
            if (reason != null) putString("reason", reason)
        }
    }
}
