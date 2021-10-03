package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageManager
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.su.SuPolicy

class SuRequestHandler(
    private val pm: PackageManager,
    private val policyDB: PolicyDao
) {
    lateinit var policy: SuPolicy
        private set

    // Return true to indicate undetermined policy, require user interaction
    suspend fun start(intent: Intent): Boolean {
        return false
    }

    fun respond(action: Int, time: Int) {
        policy.policy = action
    }
}
