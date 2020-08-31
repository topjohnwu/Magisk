package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.os.Build.VERSION.SDK_INT
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.AXML
import com.topjohnwu.magisk.core.utils.Keygen
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.signing.JarMap
import com.topjohnwu.signing.SignApk
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.security.SecureRandom

object PatchAPK {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyz"
    private const val ALPHADOTS = "$ALPHA....."

    private const val APP_ID = "com.topjohnwu.magisk"
    private const val APP_NAME = "Magisk Manager"

    // Some arbitrary limit
    const val MAX_LABEL_LENGTH = 32

    private fun genPackageName(): CharSequence {
        val random = SecureRandom()
        val len = 5 + random.nextInt(15)
        val builder = StringBuilder(len)
        var next: Char
        var prev = 0.toChar()
        for (i in 0 until len) {
            next = if (prev == '.' || i == len - 1) {
                ALPHA[random.nextInt(ALPHA.length)]
            } else {
                ALPHADOTS[random.nextInt(ALPHADOTS.length)]
            }
            builder.append(next)
            prev = next
        }
        if (!builder.contains('.')) {
            // Pick a random index and set it as dot
            val idx = random.nextInt(len - 1)
            builder[idx] = '.'
        }
        return builder
    }

    fun patch(
        context: Context,
        apk: String, out: String,
        pkg: CharSequence, label: CharSequence
    ): Boolean {
        try {
            val jar = JarMap.open(apk)
            val je = jar.getJarEntry(Const.ANDROID_MANIFEST)
            val xml = AXML(jar.getRawData(je))

            if (!xml.findAndPatch(APP_ID to pkg.toString(), APP_NAME to label.toString()))
                return false

            // Write apk changes
            jar.getOutputStream(je).write(xml.bytes)
            val keys = Keygen(context)
            SignApk.sign(keys.cert, keys.key, jar, FileOutputStream(out))
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        return true
    }

    private suspend fun patchAndHide(context: Context, label: String): Boolean {
        val dlStub = !isRunningAsStub && SDK_INT >= 28 && Const.Version.atLeast_20_2()
        val src = if (dlStub) {
            val stub = File(context.cacheDir, "stub.apk")
            val svc = get<GithubRawServices>()
            try {
                svc.fetchFile(Info.remote.stub.link).byteStream().use {
                    it.writeTo(stub)
                }
            } catch (e: IOException) {
                Timber.e(e)
                return false
            }
            stub.path
        } else {
            context.packageCodePath
        }

        // Generate a new random package name and signature
        val repack = File(context.cacheDir, "patched.apk")
        val pkg = genPackageName()
        Config.keyStoreRaw = ""

        if (!patch(context, src, repack.path, pkg, label))
            return false

        // Install the application
        if (!Shell.su("adb_pm_install $repack").exec().isSuccess)
            return false

        Config.suManager = pkg.toString()
        Config.export()
        Shell.su("pm uninstall $APP_ID").submit()

        return true
    }

    fun hideManager(context: Context, label: String) {
        val progress = Notifications.progress(context, context.getString(R.string.hide_manager_title))
        Notifications.mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build())
        GlobalScope.launch {
            val result = withContext(Dispatchers.IO) {
                patchAndHide(context, label)
            }
            if (!result)
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG)
            Notifications.mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID)
        }
    }
}
