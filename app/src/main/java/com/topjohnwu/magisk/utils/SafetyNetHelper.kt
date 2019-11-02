package com.topjohnwu.magisk.utils

interface SafetyNetHelper {

    val version: Int

    fun attest()

    interface Callback {
        fun onResponse(responseCode: Int)
    }

    companion object {

        val RESPONSE_ERR = 0x01
        val CONNECTION_FAIL = 0x02

        val BASIC_PASS = 0x10
        val CTS_PASS = 0x20
    }
}
