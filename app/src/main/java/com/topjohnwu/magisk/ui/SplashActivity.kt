package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Bundle
import android.text.TextUtils
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.tasks.UpdateRepos
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import org.koin.android.ext.android.get

open class SplashActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Shell.getShell {
            if (Config.magiskVersionCode > 0 && Config.magiskVersionCode < Const.MagiskVersion.MIN_SUPPORT) {
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
        val pkg = Config.get<String>(Config.Key.SU_MANAGER)
        if (pkg != null && packageName == BuildConfig.APPLICATION_ID) {
            Config.remove(Config.Key.SU_MANAGER)
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
        Utils.scheduleUpdateCheck()
        //CheckUpdates.check()

        // Setup shortcuts
        Shortcuts.setup(this)

        // Magisk working as expected
        if (Shell.rootAccess() && Config.magiskVersionCode > 0) {
            // Load modules
            //Utils.loadModules(false)
            // Load repos
            if (Networking.checkNetworkStatus(this)) {
                get<UpdateRepos>().exec().subscribeK()
            }
        }

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
