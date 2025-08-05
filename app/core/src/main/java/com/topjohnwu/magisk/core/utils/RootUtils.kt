package com.topjohnwu.magisk.core.utils

import android.app.ActivityManager
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.IBinder
import android.system.Os
import androidx.core.content.getSystemService
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.ipc.RootService
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.util.concurrent.locks.AbstractQueuedSynchronizer

class RootUtils(stub: Any?) : RootService() {

    private val className: String = stub?.javaClass?.name ?: javaClass.name
    private lateinit var am: ActivityManager

    constructor() : this(null)

    init {
        Timber.plant(object : Timber.DebugTree() {
            override fun log(priority: Int, tag: String?, message: String, t: Throwable?) {
                super.log(priority, "Magisk", message, t)
            }
        })
    }

    override fun onCreate() {
        am = getSystemService()!!
    }

    override fun getComponentName(): ComponentName {
        return ComponentName(packageName, className)
    }

    override fun onBind(intent: Intent): IBinder {
        return object : IRootUtils.Stub() {
            override fun getAppProcess(pid: Int) = safe(null) { getAppProcessImpl(pid) }
            override fun getFileSystem(): IBinder = FileSystemManager.getService()
            override fun addSystemlessHosts() = safe(false) { addSystemlessHostsImpl() }
        }
    }

    private fun getAppProcessImpl(_pid: Int): ActivityManager.RunningAppProcessInfo? {
        val procList = am.runningAppProcesses
        var pid = _pid
        while (pid > 1) {
            val proc = procList.find { it.pid == pid }
            if (proc != null)
                return proc

            // Stop find when root process
            if (Os.stat("/proc/$pid").st_uid == 0) {
                return null
            }

            // Find PPID
            File("/proc/$pid/status").useLines {
                val line = it.find { l -> l.startsWith("PPid:") } ?: return null
                pid = line.substring(5).trim().toInt()
            }
        }
        return null
    }

    private fun addSystemlessHostsImpl(): Boolean {
        val module = File(Const.MODULE_PATH, "hosts")
        if (module.exists()) return true
        val hosts = File(module, "system/etc/hosts")
        if (!hosts.parentFile.mkdirs()) return false
        File(module, "module.prop").outputStream().writer().use {
            it.write("""
                id=hosts
                name=Systemless Hosts
                version=1.0
                versionCode=1
                author=Magisk
                description=Magisk app built-in systemless hosts module
            """.trimIndent())
        }
        File("/system/etc/hosts").copyTo(hosts)
        File(module, "update").createNewFile()
        return true
    }

    object Connection : AbstractQueuedSynchronizer(), ServiceConnection {
        init {
            state = 1
        }

        override fun onServiceConnected(name: ComponentName, service: IBinder) {
            Timber.d("onServiceConnected")
            IRootUtils.Stub.asInterface(service).let {
                obj = it
                fs = FileSystemManager.getRemote(it.fileSystem)
            }
            releaseShared(1)
        }

        override fun onServiceDisconnected(name: ComponentName) {
            state = 1
            obj = null
            bind(Intent().setComponent(name), this)
        }

        override fun tryAcquireShared(acquires: Int) = if (state == 0) 1 else -1

        override fun tryReleaseShared(releases: Int): Boolean {
            // Decrement count; signal when transition to zero
            while (true) {
                val c = state
                if (c == 0)
                    return false
                val n = c - 1
                if (compareAndSetState(c, n))
                    return n == 0
            }
        }

        fun await() {
            if (!Info.isRooted)
                return
            if (!ShellUtils.onMainThread()) {
                acquireSharedInterruptibly(1)
            } else if (state != 0) {
                throw IllegalStateException("Cannot await on the main thread")
            }
        }
    }

    companion object {
        var bindTask: Shell.Task? = null
        var fs: FileSystemManager = FileSystemManager.getLocal()
            get() {
                Connection.await()
                return field
            }
            private set
        private var obj: IRootUtils? = null
            get() {
                Connection.await()
                return field
            }

        fun getAppProcess(pid: Int) = safe(null) { obj?.getAppProcess(pid) }

        suspend fun addSystemlessHosts() =
            withContext(Dispatchers.IO) { safe(false) { obj?.addSystemlessHosts() ?: false } }

        private inline fun <T> safe(default: T, block: () -> T): T {
            return try {
                block()
            } catch (e: Throwable) {
                // The process died unexpectedly
                Timber.e(e)
                default
            }
        }
    }
}
