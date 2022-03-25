package com.topjohnwu.magisk.core.magiskdb

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.model.su.createPolicy
import com.topjohnwu.magisk.di.AppContext
import timber.log.Timber
import java.util.concurrent.TimeUnit

class PolicyDao : MagiskDB() {

    suspend fun deleteOutdated() {
        val nowSeconds = TimeUnit.MILLISECONDS.toSeconds(System.currentTimeMillis())
        val query = "DELETE FROM ${Table.POLICY} WHERE " +
            "(until > 0 AND until < $nowSeconds) OR until < 0"
        exec(query)
    }

    suspend fun delete(uid: Int) {
        val query = "DELETE FROM ${Table.POLICY} WHERE uid == $uid"
        exec(query)
    }

    suspend fun fetch(uid: Int): SuPolicy? {
        val query = "SELECT * FROM ${Table.POLICY} WHERE uid == $uid LIMIT = 1"
        return exec(query) { it.toPolicyOrNull() }.firstOrNull()
    }

    suspend fun update(policy: SuPolicy) {
        val query = "REPLACE INTO ${Table.POLICY} ${policy.toMap().toQuery()}"
        exec(query)
    }

    suspend fun fetchAll(): List<SuPolicy> {
        val query = "SELECT * FROM ${Table.POLICY} WHERE uid/100000 == ${Const.USER_ID}"
        return exec(query) { it.toPolicyOrNull() }.filterNotNull()
    }

    private suspend fun Map<String, String>.toPolicyOrNull(): SuPolicy? {
        try {
            return AppContext.packageManager.createPolicy(this)
        } catch (e: Exception) {
            Timber.w(e)
            val uid = get("uid") ?: return null
            delete(uid.toInt())
            return null
        }
    }

}
