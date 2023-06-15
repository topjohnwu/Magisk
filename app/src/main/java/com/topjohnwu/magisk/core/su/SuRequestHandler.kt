package com.topjohnwu.magisk.core.su

import android.content.Intent
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.ktx.getPackageInfo
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.DataOutputStream
import java.io.FileOutputStream
import java.io.IOException
import java.util.concurrent.TimeUnit

class SuRequestHandler(
    val pm: PackageManager,
    private val policyDB: PolicyDao
) {

    private lateinit var output: DataOutputStream
    private lateinit var policy: SuPolicy
    lateinit var pkgInfo: PackageInfo
        private set

    // Return true to indicate undetermined policy, require user interaction
    suspend fun start(intent: Intent): Boolean {
        if (!init(intent))
            return false

        // Never allow com.topjohnwu.magisk (could be malware)
        if (pkgInfo.packageName == BuildConfig.APPLICATION_ID) {
            Shell.cmd("(pm uninstall ${BuildConfig.APPLICATION_ID} >/dev/null 2>&1)&").exec()
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

    private fun close() {
        if (::output.isInitialized)
            runCatching { output.close() }
    }

    private suspend fun init(intent: Intent): Boolean {
        val uid = intent.getIntExtra("uid", -1)
        if (uid <= 0) {
            return false;
        }
        policy = SuPolicy(uid)
        val pid = intent.getIntExtra("pid", -1)
        if (pid <= 0) {
            return false;
        }
        val fifo = intent.getStringExtra("fifo") ?: "/dev/socket/magisk_su_request_$pid"

        try {
            output = DataOutputStream(FileOutputStream(fifo))
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
            return true
        } catch (e: IOException) {
            Timber.e(e)
            close()
            return false
        }
    }

    suspend fun respond(action: Int, time: Int) {
        val until = if (time > 0)
            TimeUnit.MILLISECONDS.toSeconds(System.currentTimeMillis()) +
                TimeUnit.MINUTES.toSeconds(time.toLong())
        else
            time.toLong()

        policy.policy = action
        policy.until = until

        withContext(Dispatchers.IO) {
            try {
                output.writeInt(policy.policy)
                output.flush()
            } catch (e: IOException) {
                Timber.e(e)
            } finally {
                close()
                if (until >= 0)
                    policyDB.update(policy)
            }
        }
    }
}
