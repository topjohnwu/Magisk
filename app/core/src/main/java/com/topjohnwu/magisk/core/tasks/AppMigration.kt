package com.topjohnwu.magisk.core.tasks

import android.app.Activity
import android.app.ActivityOptions
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.widget.Toast
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.AppApkPath
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.await
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.signing.JarMap
import com.topjohnwu.magisk.core.signing.SignApk
import com.topjohnwu.magisk.core.utils.AXML
import com.topjohnwu.magisk.core.utils.Keygen
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.IOException
import java.io.OutputStream
import java.security.SecureRandom
import kotlin.random.asKotlinRandom

object AppMigration {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyz"
    private const val ALPHADOTS = "$ALPHA....."
    private const val ANDROID_MANIFEST = "AndroidManifest.xml"
    private const val TEST_PKG_NAME = "$APP_PACKAGE_NAME.test"

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
        val pm = context.packageManager
        val info = pm.getPackageArchiveInfo(apk.path, 0)?.applicationInfo ?: return false
        val origLabel = info.nonLocalizedLabel.toString()
        try {
            JarMap.open(apk, true).use { jar ->
                val je = jar.getJarEntry(ANDROID_MANIFEST)
                val xml = AXML(jar.getRawData(je))
                val generator = classNameGenerator()
                val p = xml.patchStrings {
                    when {
                        it.contains(APP_PACKAGE_NAME) -> it.replace(APP_PACKAGE_NAME, pkg)
                        it.contains(PLACEHOLDER) -> generator.next()
                        it == origLabel -> label.toString()
                        else -> it
                    }
                }
                if (!p) return false

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

    private fun patchTest(apk: File, out: File, pkg: String): Boolean {
        try {
            JarMap.open(apk, true).use { jar ->
                val je = jar.getJarEntry(ANDROID_MANIFEST)
                val xml = AXML(jar.getRawData(je))
                val p = xml.patchStrings {
                    when (it) {
                        APP_PACKAGE_NAME -> pkg
                        TEST_PKG_NAME -> "$pkg.test"
                        else -> it
                    }
                }
                if (!p) return false

                // Write apk changes
                jar.getOutputStream(je).use { it.write(xml.bytes) }
                val keys = Keygen()
                out.outputStream().use { SignApk.sign(keys.cert, keys.key, jar, it) }
                return true
            }
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }
    }

    private fun launchApp(context: Context, pkg: String) {
        val intent = context.packageManager.getLaunchIntentForPackage(pkg) ?: return
        intent.putExtra(Const.Key.PREV_CONFIG, Config.toBundle())
        val options = ActivityOptions.makeBasic()
        if (Build.VERSION.SDK_INT >= 34) {
            options.setShareIdentityEnabled(true)
        }
        context.startActivity(intent, options.toBundle())
        if (context is Activity) {
            context.finish()
        }
    }

    suspend fun patchAndHide(context: Context, label: String, pkg: String? = null): Boolean {
        val stub = File(context.cacheDir, "stub.apk")
        try {
            context.assets.open("stub.apk").writeTo(stub)
        } catch (e: IOException) {
            Timber.e(e)
            return false
        }

        // Generate a new random signature and package name if needed
        val pkg = pkg ?: genPackageName()
        Config.keyStoreRaw = ""

        // Check and patch the test APK
        try {
            val info = context.packageManager.getApplicationInfo(TEST_PKG_NAME, 0)
            val testApk = File(info.sourceDir)
            val testRepack = File(context.cacheDir, "test.apk")
            if (!patchTest(testApk, testRepack, pkg))
                return false
            val cmd = "adb_pm_install $testRepack $pkg.test"
            if (!Shell.cmd(cmd).exec().isSuccess)
                return false
        } catch (e: PackageManager.NameNotFoundException) {
        }

        val repack = File(context.cacheDir, "patched.apk")
        repack.outputStream().use {
            if (!patch(context, stub, it, pkg, label))
                return false
        }

        // Install and auto launch app
        val cmd = "adb_pm_install $repack $pkg"
        if (Shell.cmd(cmd).exec().isSuccess) {
            Config.suManager = pkg
            Shell.cmd("touch $AppApkPath").exec()
            launchApp(context, pkg)
            return true
        } else {
            return false
        }
    }

    @Suppress("DEPRECATION")
    suspend fun hide(activity: Activity, label: String) {
        val dialog = android.app.ProgressDialog(activity).apply {
            setTitle(activity.getString(R.string.hide_app_title))
            isIndeterminate = true
            setCancelable(false)
            show()
        }
        val success = withContext(Dispatchers.IO) {
            patchAndHide(activity, label)
        }
        if (!success) {
            dialog.dismiss()
            activity.toast(R.string.failure, Toast.LENGTH_LONG)
        }
    }

    suspend fun restoreApp(context: Context): Boolean {
        val apk = StubApk.current(context)
        val cmd = "adb_pm_install $apk $APP_PACKAGE_NAME"
        if (Shell.cmd(cmd).await().isSuccess) {
            Config.suManager = ""
            Shell.cmd("touch $AppApkPath").exec()
            launchApp(context, APP_PACKAGE_NAME)
            return true
        }
        return false
    }

    @Suppress("DEPRECATION")
    suspend fun restore(activity: Activity) {
        val dialog = android.app.ProgressDialog(activity).apply {
            setTitle(activity.getString(R.string.restore_img_msg))
            isIndeterminate = true
            setCancelable(false)
            show()
        }
        if (!restoreApp(activity)) {
            activity.toast(R.string.failure, Toast.LENGTH_LONG)
        }
        dialog.dismiss()
    }

    suspend fun upgradeStub(context: Context, apk: File): Intent? {
        val label = context.applicationInfo.nonLocalizedLabel
        val pkg = context.packageName
        val session = APKInstall.startSession(context)
        return withContext(Dispatchers.IO) {
            session.openStream(context).use {
                if (!patch(context, apk, it, pkg, label)) {
                    return@withContext null
                }
            }
            session.waitIntent()
        }
    }
}
