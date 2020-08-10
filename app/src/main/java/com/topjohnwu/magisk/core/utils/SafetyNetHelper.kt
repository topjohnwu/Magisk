package com.topjohnwu.magisk.core.utils

import org.json.JSONObject

interface SafetyNetHelper {

    val version: Int

    fun attest()

    interface Callback {
        fun onResponse(response: JSONObject?)
    }
}
