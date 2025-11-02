package com.topjohnwu.magisk.core.logging

import org.json.JSONObject
import java.time.Instant

object JSONLogger {
    private fun emit(obj: JSONObject) {
        // In Android use Log.i/w/e; here we print so CI/adb logcat captures it.
        println(obj.toString())
    }

    fun info(component: String, event: String, sessionId: String? = null, extra: Map<String, Any?> = emptyMap()) {
        val obj = JSONObject()
        obj.put("ts", Instant.now().toString())
        obj.put("level", "INFO")
        obj.put("component", component)
        obj.put("event", event)
        sessionId?.let { obj.put("sessionId", it) }
        obj.put("extra", JSONObject(extra))
        emit(obj)
    }

    fun error(component: String, event: String, sessionId: String? = null, extra: Map<String, Any?> = emptyMap()) {
        val obj = JSONObject()
        obj.put("ts", Instant.now().toString())
        obj.put("level", "ERROR")
        obj.put("component", component)
        obj.put("event", event)
        sessionId?.let { obj.put("sessionId", it) }
        obj.put("extra", JSONObject(extra))
        emit(obj)
    }
}
