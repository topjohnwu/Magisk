package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageManager
import android.net.LocalServerSocket
import android.net.LocalSocket
import android.net.LocalSocketAddress
import androidx.collection.ArrayMap
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.toPolicy
import com.topjohnwu.magisk.extensions.now
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import timber.log.Timber
import java.io.*
import java.util.concurrent.Callable
import java.util.concurrent.TimeUnit

abstract class SuRequestHandler(
    private val packageManager: PackageManager,
    private val policyDB: PolicyDao
) {
    private lateinit var socket: LocalSocket
    private lateinit var output: DataOutputStream
    private lateinit var input: DataInputStream

    protected lateinit var policy: MagiskPolicy
        private set

    abstract fun onStart()

    fun start(intent: Intent): Boolean {
        val name = intent.getStringExtra("socket") ?: return false

        try {
            if (Const.Version.atLeastCanary()) {
                val server = LocalServerSocket(name)
                val futureSocket = Shell.EXECUTOR.submit(Callable { server.accept() })
                try {
                    socket = futureSocket.get(1, TimeUnit.SECONDS)
                } catch (e: Exception) {
                    // Timeout or any IO errors
                    throw e
                } finally {
                    server.close()
                }
            } else {
                socket = LocalSocket()
                socket.connect(LocalSocketAddress(name, LocalSocketAddress.Namespace.ABSTRACT))
            }
            output = DataOutputStream(BufferedOutputStream(socket.outputStream))
            input = DataInputStream(BufferedInputStream(socket.inputStream))
            val map = Shell.EXECUTOR.submit(Callable { readRequest() })
                .runCatching { get(1, TimeUnit.SECONDS) }.getOrNull() ?: return false
            val uid = map["uid"]?.toIntOrNull() ?: return false
            policy = uid.toPolicy(packageManager)
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        // Never allow com.topjohnwu.magisk (could be malware)
        if (policy.packageName == BuildConfig.APPLICATION_ID)
            return false

        when (Config.suAutoReponse) {
            Config.Value.SU_AUTO_DENY -> {
                respond(MagiskPolicy.DENY, 0)
                return true
            }
            Config.Value.SU_AUTO_ALLOW -> {
                respond(MagiskPolicy.ALLOW, 0)
                return true
            }
        }
        UiThreadHandler.run { onStart() }
        return true
    }

    fun respond(action: Int, time: Int) {
        val until = if (time > 0)
            TimeUnit.MILLISECONDS.toSeconds(now) + TimeUnit.MINUTES.toSeconds(time.toLong())
        else
            time.toLong()

        policy.policy = action
        policy.until = until
        policy.uid = policy.uid % 100000 + Const.USER_ID * 100000

        Shell.EXECUTOR.submit {
            try {
                output.writeInt(policy.policy)
                output.flush()
            } catch (e: IOException) {
                Timber.e(e)
            } finally {
                if (until >= 0)
                    policyDB.update(policy).blockingAwait()
                runCatching {
                    input.close()
                    output.close()
                    socket.close()
                }
            }
        }
    }

    @Throws(IOException::class)
    private fun readRequest(): Map<String, String> {
        fun readString(): String {
            val len = input.readInt()
            val buf = ByteArray(len)
            input.readFully(buf)
            return String(buf, Charsets.UTF_8)
        }
        val ret = ArrayMap<String, String>()
        while (true) {
            val name = readString()
            if (name == "eof")
                break
            ret[name] = readString()
        }
        return ret
    }

}
