package com.topjohnwu.magisk.core.magiskdb

import android.content.Context
import android.content.pm.PackageManager
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.toMap
import com.topjohnwu.magisk.core.model.toPolicy
import com.topjohnwu.magisk.extensions.now
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import timber.log.Timber
import java.util.concurrent.TimeUnit


class PolicyDao(
    private val context: Context
) : BaseDao() {

    override val table: String = Table.POLICY

    suspend fun deleteOutdated() = buildQuery<Delete> {
        condition {
            greaterThan("until", "0")
            and {
                lessThan("until", TimeUnit.MILLISECONDS.toSeconds(now).toString())
            }
            or {
                lessThan("until", "0")
            }
        }
    }.commit()

    suspend fun delete(packageName: String) = buildQuery<Delete> {
        condition {
            equals("package_name", packageName)
        }
    }.commit()

    suspend fun delete(uid: Int) = buildQuery<Delete> {
        condition {
            equals("uid", uid)
        }
    }.commit()

    suspend fun fetch(uid: Int) = buildQuery<Select> {
        condition {
            equals("uid", uid)
        }
    }.query().first().toPolicyOrNull()

    suspend fun update(policy: MagiskPolicy) = buildQuery<Replace> {
        values(policy.toMap())
    }.commit()

    suspend fun <R: Any> fetchAll(mapper: (MagiskPolicy) -> R) = buildQuery<Select> {
        condition {
            equals("uid/100000", Const.USER_ID)
        }
    }.query {
        it.toPolicyOrNull()?.let(mapper)
    }

    private fun Map<String, String>.toPolicyOrNull(): MagiskPolicy? {
        return runCatching { toPolicy(context.packageManager) }.getOrElse {
            Timber.e(it)
            if (it is PackageManager.NameNotFoundException) {
                val uid = getOrElse("uid") { null } ?: return null
                GlobalScope.launch {
                    delete(uid.toInt())
                }
            }
            null
        }
    }

}
