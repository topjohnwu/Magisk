package com.topjohnwu.magisk.ui.safetynet

import org.json.JSONObject

interface SafetyNetHelper {

    val version: Int

    fun attest()

    interface Callback {
        fun onResponse(response: JSONObject?)
    }
}
