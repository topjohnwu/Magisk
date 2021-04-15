package com.topjohnwu.magisk.ui.safetynet

interface SafetyNetHelper {

    val version: Int

    fun attest(nonce: ByteArray)

    interface Callback {
        fun onResponse(response: String?)
    }
}
