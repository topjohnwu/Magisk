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
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.model.su.toPolicy
import com.topjohnwu.magisk.ktx.now
import kotlinx.coroutines.*
import timber.log.Timber
import java.io.*
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeUnit.SECONDS

abstract class SuRequestHandler(
    private val packageManager: PackageManager,
    private val policyDB: PolicyDao
) {
    private lateinit var socket: LocalSocket
    private lateinit var output: DataOutputStream
    private lateinit var input: DataInputStream

    protected lateinit var policy: SuPolicy
        private set

    abstract fun onStart()

    suspend fun start(intent: Intent): Boolean {
        val name = intent.getStringExtra("socket") ?: return false

        if (!init(name))
            return false

        // Never allow com.topjohnwu.magisk (could be malware)
        if (policy.packageName == BuildConfig.APPLICATION_ID)
            return false

        when (Config.suAutoReponse) {
            Config.Value.SU_AUTO_DENY -> {
                respond(SuPolicy.DENY, 0)
                return true
            }
            Config.Value.SU_AUTO_ALLOW -> {
                respond(SuPolicy.ALLOW, 0)
                return true
            }
        }

        onStart()
        return true
    }

    private suspend fun <T> Deferred<T>.timedAwait() : T? {
        return withTimeoutOrNull(SECONDS.toMillis(1)) {
            await()
        }
    }

    private class SocketError : IOException()

    private suspend fun init(name: String) = withContext(Dispatchers.IO) {
        try {
            if (Const.Version.atLeastCanary()) {
                val server = LocalServerSocket(name)
                // Do NOT use Closable?.use(block) here as LocalServerSocket does
                // not implement Closable on older Android platforms
                try {
                    socket = async { server.accept() }.timedAwait() ?: throw SocketError()
                } finally {
                    server.close()
                }
            } else {
                socket = LocalSocket()
                socket.connect(LocalSocketAddress(name, LocalSocketAddress.Namespace.ABSTRACT))
            }
            output = DataOutputStream(BufferedOutputStream(socket.outputStream))
            input = DataInputStream(BufferedInputStream(socket.inputStream))
            val map = async { readRequest() }.timedAwait() ?: throw SocketError()
            val uid = map["uid"]?.toIntOrNull() ?: throw SocketError()
            policy = uid.toPolicy(packageManager)
            true
        } catch (e: Exception) {
            when (e) {
                is IOException, is PackageManager.NameNotFoundException -> {
                    Timber.e(e)
                    if (::socket.isInitialized)
                        socket.close()
                    false
                }
                else -> throw e  // Unexpected error
            }
        }
    }

    fun respond(action: Int, time: Int) {
        val until = if (time > 0)
            TimeUnit.MILLISECONDS.toSeconds(now) + TimeUnit.MINUTES.toSeconds(time.toLong())
        else
            time.toLong()

        policy.policy = action
        policy.until = until
        policy.uid = policy.uid % 100000 + Const.USER_ID * 100000

        GlobalScope.launch(Dispatchers.IO) {
            try {
                output.writeInt(policy.policy)
                output.flush()
            } catch (e: IOException) {
                Timber.e(e)
            } finally {
                runCatching {
                    input.close()
                    output.close()
                    socket.close()
                }
                if (until >= 0)
                    policyDB.update(policy)
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
