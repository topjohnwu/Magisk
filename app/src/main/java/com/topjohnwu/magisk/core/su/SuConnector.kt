package com.topjohnwu.magisk.core.su

import android.net.LocalSocket
import android.net.LocalSocketAddress
import androidx.collection.ArrayMap
import timber.log.Timber
import java.io.*

abstract class SuConnector @Throws(IOException::class)
protected constructor(name: String) {

    private val socket: LocalSocket = LocalSocket()
    protected var out: DataOutputStream
    protected var input: DataInputStream

    init {
        socket.connect(LocalSocketAddress(name, LocalSocketAddress.Namespace.ABSTRACT))
        out = DataOutputStream(BufferedOutputStream(socket.outputStream))
        input = DataInputStream(BufferedInputStream(socket.inputStream))
    }

    private fun readString(): String {
        val len = input.readInt()
        val buf = ByteArray(len)
        input.readFully(buf)
        return String(buf, Charsets.UTF_8)
    }

    @Throws(IOException::class)
    fun readRequest(): Map<String, String> {
        val ret = ArrayMap<String, String>()
        while (true) {
            val name = readString()
            if (name == "eof")
                break
            ret[name] = readString()
        }
        return ret
    }

    fun response() {
        runCatching {
            onResponse()
            out.flush()
        }.onFailure { Timber.e(it) }

        runCatching {
            input.close()
            out.close()
            socket.close()
        }
    }

    @Throws(IOException::class)
    protected abstract fun onResponse()

}
