package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.ktx.getPackageInfo
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.DataOutputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.util.concurrent.TimeUnit

class SuRequestHandler(
    val pm: PackageManager,
    private val policyDB: PolicyDao
) {

    private lateinit var output: File
    private lateinit var policy: SuPolicy
    lateinit var pkgInfo: PackageInfo
        private set

    // Return true to indicate undetermined policy, require user interaction
    suspend fun start(intent: Intent): Boolean {
        if (!init(intent))
            return false

        // Never allow com.topjohnwu.magisk (could be malware)
        if (pkgInfo.packageName == BuildConfig.APP_PACKAGE_NAME) {
            Shell.cmd("(pm uninstall ${BuildConfig.APP_PACKAGE_NAME} >/dev/null 2>&1)&").exec()
            return false
        }

        when (Config.suAutoResponse) {
            Config.Value.SU_AUTO_DENY -> {
                respond(SuPolicy.DENY, 0)
                return false
            }
            Config.Value.SU_AUTO_ALLOW -> {
                respond(SuPolicy.ALLOW, 0)
                return false
            }
        }

        return true
    }

    private suspend fun init(intent: Intent): Boolean {
        val uid = intent.getIntExtra("uid", -1)
        val pid = intent.getIntExtra("pid", -1)
        val fifo = intent.getStringExtra("fifo")
        if (uid <= 0 || pid <= 0 || fifo == null) {
            Timber.e("Unexpected extras: uid=[${uid}], pid=[${pid}], fifo=[${fifo}]")
            return false
        }
        output = File(fifo)
        policy = policyDB.fetch(uid) ?: SuPolicy(uid)
        try {
            pkgInfo = pm.getPackageInfo(uid, pid) ?: PackageInfo().apply {
                val name = pm.getNameForUid(uid) ?: throw PackageManager.NameNotFoundException()
                // We only fill in sharedUserId and leave other fields uninitialized
                sharedUserId = name.split(":")[0]
            }
        } catch (e: PackageManager.NameNotFoundException) {
            Timber.e(e)
            respond(SuPolicy.DENY, -1)
            return false
        }
        if (!output.canWrite()) {
            Timber.e("Cannot write to $output")
            return false
        }
        return true
    }

    suspend fun respond(action: Int, time: Long) {
        if (action == SuPolicy.ALLOW && Config.suRestrict) {
            policy.policy = SuPolicy.RESTRICT
        } else {
            policy.policy = action
        }
        if (time >= 0) {
            policy.remain = TimeUnit.MINUTES.toSeconds(time)
        } else {
            policy.remain = time
        }

        withContext(Dispatchers.IO) {
            try {
                DataOutputStream(FileOutputStream(output)).use {
                    it.writeInt(policy.policy)
                    it.flush()
                }
            } catch (e: IOException) {
                Timber.e(e)
            }
            if (time >= 0) {
                policyDB.update(policy)
            }
        }
    }
}
