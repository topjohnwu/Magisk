package com.topjohnwu.magisk.core.data.magiskdb

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.di.AppContext
import com.topjohnwu.magisk.core.model.su.SuPolicy
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
        return exec(query, ::toPolicy).firstOrNull()
    }

    suspend fun update(policy: SuPolicy) {
        val map = policy.toMap()
        if (!Const.Version.atLeast_25_0()) {
            // Put in package_name for old database
            map["package_name"] = AppContext.packageManager.getNameForUid(policy.uid)!!
        }
        val query = "REPLACE INTO ${Table.POLICY} ${map.toQuery()}"
        exec(query)
    }

    suspend fun fetchAll(): List<SuPolicy> {
        val query = "SELECT * FROM ${Table.POLICY} WHERE uid/100000 == ${Const.USER_ID}"
        return exec(query, ::toPolicy).filterNotNull()
    }

    private fun toPolicy(map: Map<String, String>): SuPolicy? {
        val uid = map["uid"]?.toInt() ?: return null
        val policy = SuPolicy(uid)

        map["policy"]?.toInt()?.let { policy.policy = it }
        map["until"]?.toLong()?.let { policy.until = it }
        map["logging"]?.toInt()?.let { policy.logging = it != 0 }
        map["notification"]?.toInt()?.let { policy.notification = it != 0 }
        return policy
    }

}
