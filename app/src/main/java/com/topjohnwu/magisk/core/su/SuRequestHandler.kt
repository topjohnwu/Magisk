package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageManager
import android.net.LocalSocket
import android.net.LocalSocketAddress
import android.os.CountDownTimer
import androidx.collection.ArrayMap
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.toPolicy
import com.topjohnwu.magisk.extensions.now
import timber.log.Timber
import java.io.*
import java.util.concurrent.TimeUnit

abstract class SuRequestHandler(
    private val packageManager: PackageManager,
    private val policyDB: PolicyDao
) {
    private val socket: LocalSocket = LocalSocket()
    private lateinit var out: DataOutputStream
    private lateinit var input: DataInputStream

    protected var timer: CountDownTimer = DefaultCountDown()
        set(value) {
            field.cancel()
            field = value
            field.start()
        }
    protected lateinit var policy: MagiskPolicy
        private set

    abstract fun onStart()

    fun start(intent: Intent): Boolean {
        val socketName = intent.getStringExtra("socket") ?: return false

        try {
            socket.connect(LocalSocketAddress(socketName, LocalSocketAddress.Namespace.ABSTRACT))
            out = DataOutputStream(BufferedOutputStream(socket.outputStream))
            input = DataInputStream(BufferedInputStream(socket.inputStream))
            val map = readRequest()
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

        timer.start()
        onStart()
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

        if (until >= 0)
            policyDB.update(policy).blockingAwait()

        try {
            out.writeInt(policy.policy)
            out.flush()
        } catch (e: IOException) {
            Timber.e(e)
        } finally {
            runCatching {
                input.close()
                out.close()
                socket.close()
            }
        }

        timer.cancel()
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

    private inner class DefaultCountDown
        : CountDownTimer(TimeUnit.MINUTES.toMillis(1), TimeUnit.MINUTES.toMillis(1)) {
        override fun onFinish() {
            respond(MagiskPolicy.DENY, 0)
        }
        override fun onTick(remains: Long) {}
    }
}
