package com.topjohnwu.magisk.utils

import android.net.LocalSocket
import android.net.LocalSocketAddress
import android.os.Bundle
import android.text.TextUtils
import timber.log.Timber
import java.io.*
import java.nio.charset.Charset

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

    @Throws(IOException::class)
    private fun readString(): String {
        val len = input.readInt()
        val buf = ByteArray(len)
        input.readFully(buf)
        return String(buf, Charset.forName("UTF-8"))
    }

    @Throws(IOException::class)
    fun readSocketInput(): Bundle {
        val bundle = Bundle()
        while (true) {
            val name = readString()
            if (TextUtils.equals(name, "eof"))
                break
            bundle.putString(name, readString())
        }
        return bundle
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
