package com.topjohnwu.magisk.core.tasks

import android.app.ProgressDialog
import android.content.Context
import android.content.Intent
import android.widget.Toast
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.Provider
import com.topjohnwu.magisk.core.utils.AXML
import com.topjohnwu.magisk.core.utils.Keygen
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.ktx.inject
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.utils.Utils
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

object HideAPK {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyz"
    private const val ALPHADOTS = "$ALPHA....."
    private const val APP_NAME = "Magisk"
    private const val ANDROID_MANIFEST = "AndroidManifest.xml"

    // Some arbitrary limit
    const val MAX_LABEL_LENGTH = 32

    private val svc: NetworkService by inject()
    private val Context.APK_URI get() = Provider.APK_URI(packageName)
    private val Context.PREFS_URI get() = Provider.PREFS_URI(packageName)

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
            val idx = random.nextInt(len - 1)
            builder[idx] = '.'
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

    private suspend fun patchAndHide(context: Context, label: String): Boolean {
        val stub = File(context.cacheDir, "stub.apk")
        try {
            svc.fetchFile(Info.remote.stub.link).byteStream().writeTo(stub)
        } catch (e: IOException) {
            Timber.e(e)
            return false
        }

        // Generate a new random package name and signature
        val repack = File(context.cacheDir, "patched.apk")
        val pkg = genPackageName()
        Config.keyStoreRaw = ""

        if (!patch(context, stub, repack, pkg, label))
            return false

        // Install the application
        if (!Shell.su("adb_pm_install $repack").exec().isSuccess)
            return false

        context.apply {
            val intent = packageManager.getLaunchIntentForPackage(pkg) ?: return false
            Config.suManager = pkg
            grantUriPermission(pkg, APK_URI, Intent.FLAG_GRANT_READ_URI_PERMISSION)
            grantUriPermission(pkg, PREFS_URI, Intent.FLAG_GRANT_READ_URI_PERMISSION)
            intent.putExtra(Const.Key.PREV_PKG, packageName)
            startActivity(intent)
        }

        return true
    }

    @Suppress("DEPRECATION")
    fun hide(context: Context, label: String) {
        val dialog = ProgressDialog.show(context, context.getString(R.string.hide_manager_title), "", true)
        GlobalScope.launch {
            val result = withContext(Dispatchers.IO) {
                patchAndHide(context, label)
            }
            if (!result) {
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG)
                dialog.dismiss()
            }
        }
    }

    private fun restoreImpl(context: Context): Boolean {
        val apk = DynAPK.current(context)
        if (!Shell.su("adb_pm_install $apk").exec().isSuccess)
            return false

        context.apply {
            val intent = packageManager.getLaunchIntentForPackage(APPLICATION_ID) ?: return false
            Config.suManager = ""
            grantUriPermission(APPLICATION_ID, APK_URI, Intent.FLAG_GRANT_READ_URI_PERMISSION)
            grantUriPermission(APPLICATION_ID, PREFS_URI, Intent.FLAG_GRANT_READ_URI_PERMISSION)
            intent.putExtra(Const.Key.PREV_PKG, packageName)
            startActivity(intent)
        }

        return true
    }

    @Suppress("DEPRECATION")
    fun restore(context: Context) {
        val dialog = ProgressDialog.show(context, context.getString(R.string.restore_img_msg), "", true)
        GlobalScope.launch {
            val result = withContext(Dispatchers.IO) {
                restoreImpl(context)
            }
            if (!result) {
                Utils.toast(R.string.restore_manager_fail_toast, Toast.LENGTH_LONG)
                dialog.dismiss()
            }
        }
    }
}
