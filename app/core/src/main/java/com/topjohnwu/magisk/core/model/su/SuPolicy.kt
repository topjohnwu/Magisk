package com.topjohnwu.magisk.core.model.su

class SuPolicy(
    var uid: Int,
    var policy: Int = DENY,
    var logging: Boolean = true,
    var notification: Boolean = true,
) {
    companion object {
        const val QUERY = 0
        const val DENY = 1
        const val ALLOW = 2
        const val RESTRICT = 3
    }

    fun toMap(): MutableMap<String, Any> {
        return mutableMapOf(
            "uid" to uid,
            "policy" to policy,
            // Timed grants are no longer supported; all policies are permanent
            "until" to 0,
            "logging" to logging,
            "notification" to notification
        )
    }
}
