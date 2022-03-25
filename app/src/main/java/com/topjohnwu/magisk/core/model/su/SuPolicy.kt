package com.topjohnwu.magisk.core.model.su

class SuPolicy(val uid: Int) {
    companion object {
        const val INTERACTIVE = 0
        const val DENY = 1
        const val ALLOW = 2
    }

    var policy: Int = INTERACTIVE
    var until: Long = -1L
    var logging: Boolean = true
    var notification: Boolean = true

    fun toMap(): MutableMap<String, Any> = mutableMapOf(
        "uid" to uid,
        "policy" to policy,
        "until" to until,
        "logging" to logging,
        "notification" to notification
    )
}
