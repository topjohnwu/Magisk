package com.topjohnwu.magisk.data.database

import android.content.Context
import android.content.pm.PackageManager
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.data.database.base.*
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.entity.toMap
import com.topjohnwu.magisk.model.entity.toPolicy
import com.topjohnwu.magisk.utils.now
import java.util.concurrent.TimeUnit


class PolicyDao(
    private val context: Context
) : BaseDao() {

    override val table: String = DatabaseDefinition.Table.POLICY

    fun deleteOutdated(
        nowSeconds: Long = TimeUnit.MILLISECONDS.toSeconds(now)
    ) = query<Delete> {
        condition {
            greaterThan("until", "0")
            and {
                lessThan("until", nowSeconds.toString())
            }
            or {
                lessThan("until", "0")
            }
        }
    }.ignoreElement()

    fun delete(packageName: String) = query<Delete> {
        condition {
            equals("package_name", packageName)
        }
    }.ignoreElement()

    fun delete(uid: Int) = query<Delete> {
        condition {
            equals("uid", uid)
        }
    }.ignoreElement()

    fun fetch(uid: Int) = query<Select> {
        condition {
            equals("uid", uid)
        }
    }.map { it.firstOrNull()?.toPolicy(context.packageManager) }
        .doOnError {
            if (it is PackageManager.NameNotFoundException) {
                delete(uid).subscribe()
            }
        }

    fun update(policy: MagiskPolicy) = query<Replace> {
        values(policy.toMap())
    }.ignoreElement()

    fun fetchAll() = query<Select> {
        condition {
            equals("uid/100000", Const.USER_ID)
        }
    }.flattenAsFlowable { it }
        .map { it.toPolicy(context.packageManager) }
        .toList()

}