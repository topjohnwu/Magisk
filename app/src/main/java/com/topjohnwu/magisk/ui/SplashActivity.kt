package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Bundle
import android.text.TextUtils
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell

open class SplashActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Shell.getShell {
            if (Info.magiskVersionCode > 0 && Info.magiskVersionCode < Const.MagiskVersion.MIN_SUPPORT) {
                AlertDialog.Builder(this)
                    .setTitle(R.string.unsupport_magisk_title)
                    .setMessage(R.string.unsupport_magisk_message)
                    .setNegativeButton(R.string.ok, null)
                    .setOnDismissListener { finish() }
                    .show()
            } else {
                initAndStart()
            }
        }
    }

    private fun initAndStart() {
        val pkg = Config.suManager
        if (Config.suManager.isNotEmpty() && packageName == BuildConfig.APPLICATION_ID) {
            Config.suManager = ""
            Shell.su("pm uninstall $pkg").submit()
        }
        if (TextUtils.equals(pkg, packageName)) {
            runCatching {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                packageManager.getApplicationInfo(BuildConfig.APPLICATION_ID, 0)
                Shell.su("pm uninstall " + BuildConfig.APPLICATION_ID).submit()
            }
        }

        // Set default configs
        Config.initialize()

        // Create notification channel on Android O
        Notifications.setup(this)

        // Schedule periodic update checks
        Utils.scheduleUpdateCheck(this)

        // Setup shortcuts
        Shortcuts.setup(this)

        val intent = Intent(this, ClassMap[MainActivity::class.java])
        intent.putExtra(Const.Key.OPEN_SECTION, getIntent().getStringExtra(Const.Key.OPEN_SECTION))
        DONE = true
        startActivity(intent)
        finish()
    }

    companion object {

        var DONE = false
    }
}
