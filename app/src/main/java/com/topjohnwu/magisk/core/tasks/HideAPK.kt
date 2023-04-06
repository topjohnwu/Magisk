package com.topjohnwu.magisk.core.tasks

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.widget.Toast
import androidx.annotation.WorkerThread
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Provider
import com.topjohnwu.magisk.core.ktx.await
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.AXML
import com.topjohnwu.magisk.core.utils.Keygen
import com.topjohnwu.magisk.signing.JarMap
import com.topjohnwu.magisk.signing.SignApk
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.OutputStream
import java.security.SecureRandom
import kotlin.random.asKotlinRandom

object HideAPK {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyz"
    private const val ALPHADOTS = "$ALPHA....."
    private const val ANDROID_MANIFEST = "AndroidManifest.xml"

    // Some arbitrary limit
    const val MAX_LABEL_LENGTH = 32
    const val PLACEHOLDER = "COMPONENT_PLACEHOLDER"

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

    private fun classNameGenerator() = sequence {
        val c1 = mutableListOf<String>()
        val c2 = mutableListOf<String>()
        val c3 = mutableListOf<String>()
        val random = SecureRandom()
        val kRandom = random.asKotlinRandom()

        fun <T> chain(vararg iters: Iterable<T>) = sequence {
            iters.forEach { it.forEach { v -> yield(v) } }
        }

        for (a in chain('a'..'z', 'A'..'Z')) {
            if (a != 'a' && a != 'A') {
                c1.add("$a")
            }
            for (b in chain('a'..'z', 'A'..'Z', '0'..'9')) {
                c2.add("$a$b")
                for (c in chain('a'..'z', 'A'..'Z', '0'..'9')) {
                    c3.add("$a$b$c")
                }
            }
        }

        c1.shuffle(random)
        c2.shuffle(random)
        c3.shuffle(random)

        fun notJavaKeyword(name: String) = when (name) {
            "do", "if", "for", "int", "new", "try" -> false
            else -> true
        }

        fun List<String>.process() = asSequence().filter(::notJavaKeyword)

        val names = mutableListOf<String>()
        names.addAll(c1)
        names.addAll(c2.process().take(30))
        names.addAll(c3.process().take(30))

        while (true) {
            val seg = 2 + random.nextInt(4)
            val cls = StringBuilder()
            for (i in 0 until seg) {
                cls.append(names.random(kRandom))
                if (i != seg - 1)
                    cls.append('.')
            }
            // Old Android does not support capitalized package names
            // Check Android 7.0.0 PackageParser#buildClassName
            cls[0] = cls[0].lowercaseChar()
            yield(cls.toString())
        }
    }.distinct().iterator()

    private fun patch(
        context: Context,
        apk: File, out: OutputStream,
        pkg: String, label: CharSequence
    ): Boolean {
        val info = context.packageManager.getPackageArchiveInfo(apk.path, 0) ?: return false
        val origLabel = info.applicationInfo.nonLocalizedLabel.toString()
        try {
            JarMap.open(apk, true).use { jar ->
                val je = jar.getJarEntry(ANDROID_MANIFEST)
                val xml = AXML(jar.getRawData(je))
                val generator = classNameGenerator()

                if (!xml.patchStrings {
                    for (i in it.indices) {
                        val s = it[i]
                        if (s.contains(APPLICATION_ID)) {
                            it[i] = s.replace(APPLICATION_ID, pkg)
                        } else if (s.contains(PLACEHOLDER)) {
                            it[i] = generator.next()
                        } else if (s == origLabel) {
                            it[i] = label.toString()
                        }
                    }
                }) {
                    return false
                }

                // Write apk changes
                jar.getOutputStream(je).use { it.write(xml.bytes) }
                val keys = Keygen()
                SignApk.sign(keys.cert, keys.key, jar, out)
                return true
            }
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }
    }

    private fun launchApp(activity: Activity, pkg: String) {
        val intent = activity.packageManager.getLaunchIntentForPackage(pkg) ?: return
        val self = activity.packageName
        val flag = Intent.FLAG_GRANT_READ_URI_PERMISSION
        activity.grantUriPermission(pkg, Provider.preferencesUri(self), flag)
        intent.putExtra(Const.Key.PREV_PKG, self)
        activity.startActivity(intent)
        activity.finish()
    }

    private fun patchAndHide(activity: Activity, label: String, onFailure: Runnable): Boolean {
        val stub = File(activity.cacheDir, "stub.apk")
        try {
            activity.assets.open("stub.apk").writeTo(stub)
        } catch (e: IOException) {
            Timber.e(e)
            return false
        }

        // Generate a new random package name and signature
        val repack = File(activity.cacheDir, "patched.apk")
        val pkg = genPackageName()
        Config.keyStoreRaw = ""

        if (!patch(activity, stub, FileOutputStream(repack), pkg, label))
            return false

        // Install and auto launch app
        val session = APKInstall.startSession(activity, pkg, onFailure) {
            launchApp(activity, pkg)
        }

        Config.suManager = pkg
        val cmd = "adb_pm_install $repack $pkg"
        if (Shell.cmd(cmd).exec().isSuccess) return true

        try {
            session.install(activity, repack)
        } catch (e: IOException) {
            Timber.e(e)
            return false
        }
        session.waitIntent()?.let { activity.startActivity(it) } ?: return false
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
        val onFailure = Runnable {
            dialog.dismiss()
            activity.toast(R.string.failure, Toast.LENGTH_LONG)
        }
        val success = withContext(Dispatchers.IO) {
            patchAndHide(activity, label, onFailure)
        }
        if (!success) onFailure.run()
    }

    @Suppress("DEPRECATION")
    suspend fun restore(activity: Activity) {
        val dialog = android.app.ProgressDialog(activity).apply {
            setTitle(activity.getString(R.string.restore_img_msg))
            isIndeterminate = true
            setCancelable(false)
            show()
        }
        val onFailure = Runnable {
            dialog.dismiss()
            activity.toast(R.string.failure, Toast.LENGTH_LONG)
        }
        val apk = StubApk.current(activity)
        val session = APKInstall.startSession(activity, APPLICATION_ID, onFailure) {
            launchApp(activity, APPLICATION_ID)
            dialog.dismiss()
        }
        Config.suManager = ""
        val cmd = "adb_pm_install $apk $APPLICATION_ID"
        if (Shell.cmd(cmd).await().isSuccess) return
        val success = withContext(Dispatchers.IO) {
            try {
                session.install(activity, apk)
            } catch (e: IOException) {
                Timber.e(e)
                return@withContext false
            }
            session.waitIntent()?.let { activity.startActivity(it) } ?: return@withContext false
            return@withContext true
        }
        if (!success) onFailure.run()
    }

    @WorkerThread
    fun upgrade(context: Context, apk: File): Intent? {
        val label = context.applicationInfo.nonLocalizedLabel
        val pkg = context.packageName
        val session = APKInstall.startSession(context)
        session.openStream(context).use {
            if (!patch(context, apk, it, pkg, label)) {
                return null
            }
        }
        return session.waitIntent()
    }
}
