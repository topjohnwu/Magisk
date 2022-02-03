package com.topjohnwu.magisk.core.tasks

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.widget.Toast
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.Provider
import com.topjohnwu.magisk.core.utils.AXML
import com.topjohnwu.magisk.core.utils.Keygen
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.signing.JarMap
import com.topjohnwu.magisk.signing.SignApk
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.security.SecureRandom

object HideAPK {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyz"
    private const val ALPHADOTS = "$ALPHA....."
    private const val APP_NAME = "Magisk"
    private const val ANDROID_MANIFEST = "AndroidManifest.xml"

    // Some arbitrary limit
    const val MAX_LABEL_LENGTH = 32

    private val svc get() = ServiceLocator.networkService

    private fun genPackageName(): String {
        val random = SecureRandom()
        val len = 5 + random.nextInt(15)
        val builder = StringBuilder(len)
        var next: Char
        var prev = 0.toChar()
        for (i in 0 until len) {
            next = if (prev == '.' || i == 0 || i == len - 1) {
                ALPHA[random.nextInt(ALPHA.length)]
            } else {
                ALPHADOTS[random.nextInt(ALPHADOTS.length)]
            }
            builder.append(next)
            prev = next
        }
        if (!builder.contains('.')) {
            // Pick a random index and set it as dot
            val idx = random.nextInt(len - 2)
            builder[idx + 1] = '.'
        }
        return builder.toString()
    }

    fun patch(
        context: Context,
        apk: File, out: File,
        pkg: String, label: CharSequence
    ): Boolean {
        try {
            val jar = JarMap.open(apk, true)
            val je = jar.getJarEntry(ANDROID_MANIFEST)
            val xml = AXML(jar.getRawData(je))

            if (!xml.findAndPatch(APPLICATION_ID to pkg, APP_NAME to label.toString()))
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

    private fun launchApp(activity: Activity, pkg: String) {
        val intent = activity.packageManager.getLaunchIntentForPackage(pkg) ?: return
        Config.suManager = if (pkg == APPLICATION_ID) "" else pkg
        val self = activity.packageName
        val flag = Intent.FLAG_GRANT_READ_URI_PERMISSION
        activity.grantUriPermission(pkg, Provider.APK_URI(self), flag)
        activity.grantUriPermission(pkg, Provider.PREFS_URI(self), flag)
        intent.putExtra(Const.Key.PREV_PKG, self)
        activity.startActivity(intent)
        activity.finish()
    }

    private suspend fun patchAndHide(activity: Activity, label: String): Boolean {
        val stub = File(activity.cacheDir, "stub.apk")
        try {
            svc.fetchFile(Info.remote.stub.link).byteStream().writeTo(stub)
        } catch (e: IOException) {
            Timber.e(e)
            stub.createNewFile()
            val cmd = "\$MAGISKBIN/magiskinit -x manager ${stub.path}"
            if (!Shell.su(cmd).exec().isSuccess) return false
        }

        // Generate a new random package name and signature
        val repack = File(activity.cacheDir, "patched.apk")
        val pkg = genPackageName()
        Config.keyStoreRaw = ""

        if (!patch(activity, stub, repack, pkg, label))
            return false

        // Install and auto launch app
        val receiver = APKInstall.register(activity, pkg) {
            launchApp(activity, pkg)
        }
        val cmd = "adb_pm_install $repack ${activity.applicationInfo.uid}"
        if (!Shell.su(cmd).exec().isSuccess) {
            APKInstall.install(activity, repack)
            receiver.waitIntent()?.let { activity.startActivity(it) }
        }
        return true
    }

    @Suppress("DEPRECATION")
    suspend fun hide(activity: Activity, label: String) {
        val dialog = android.app.ProgressDialog(activity).apply {
            setTitle(activity.getString(R.string.hide_app_title))
            isIndeterminate = true
            setCancelable(false)
            show()
        }
        val result = withContext(Dispatchers.IO) {
            patchAndHide(activity, label)
        }
        if (!result) {
            dialog.dismiss()
            Utils.toast(R.string.failure, Toast.LENGTH_LONG)
        }
    }

    @Suppress("DEPRECATION")
    fun restore(activity: Activity) {
        val dialog = android.app.ProgressDialog(activity).apply {
            setTitle(activity.getString(R.string.restore_img_msg))
            isIndeterminate = true
            setCancelable(false)
            show()
        }
        val apk = StubApk.current(activity)
        val receiver = APKInstall.register(activity, APPLICATION_ID) {
            launchApp(activity, APPLICATION_ID)
            dialog.dismiss()
        }
        val cmd = "adb_pm_install $apk ${activity.applicationInfo.uid}"
        Shell.su(cmd).submit(Shell.EXECUTOR) { ret ->
            if (ret.isSuccess) return@submit
            APKInstall.install(activity, apk)
            receiver.waitIntent()?.let { activity.startActivity(it) }
        }
    }
}
