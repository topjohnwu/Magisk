package com.topjohnwu.magisk.core.utils

interface SafetyNetHelper {

    val version: Int

    fun attest()

    interface Callback {
        fun onResponse(responseCode: Int)
    }

    companion object {

        const val RESPONSE_ERR = 0x01
        const val CONNECTION_FAIL = 0x02

        const val BASIC_PASS = 0x10
        const val CTS_PASS = 0x20
    }
}
