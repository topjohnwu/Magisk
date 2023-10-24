package com.topjohnwu.magisk.core.su

import android.os.Bundle
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.NOPList
import kotlinx.coroutines.runBlocking

object TestHandler {

    fun run(method: String): Bundle {
        val r = Bundle()

        fun setup(): Boolean {
            val nop = NOPList.getInstance()
            return runBlocking {
                MagiskInstaller.Emulator(nop, nop).exec()
            }
        }

        fun test(): Boolean {
            // Make sure Zygisk works correctly
            if (!Info.isZygiskEnabled) {
                r.putString("reason", "zygisk not enabled")
                return false
            }

            // Make sure the Magisk app can get root
            val shell = Shell.getShell()
            if (!shell.isRoot) {
                r.putString("reason", "shell not root")
                return false
            }

            // Make sure the root service is running
            RootUtils.Connection.await()

            // Clear existing grant for ADB shell
            runBlocking {
                ServiceLocator.policyDB.delete(2000)
                Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
                Config.prefs.edit().commit()
            }
            return true
        }

        val b = runCatching {
            when (method) {
                "setup" -> setup()
                "test" -> test()
                else -> {
                    r.putString("reason", "unknown method")
                    false
                }
            }
        }.getOrElse {
            r.putString("reason", it.stackTraceToString())
            false
        }

        r.putBoolean("result", b)
        return r
    }
}
