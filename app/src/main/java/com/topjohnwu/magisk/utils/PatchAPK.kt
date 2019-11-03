package com.topjohnwu.magisk.utils

import android.content.Context
import android.os.Build.VERSION.SDK_INT
import android.widget.Toast
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.extensions.DynamicClassLoader
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.signing.JarMap
import com.topjohnwu.signing.SignAPK
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.security.SecureRandom

object PatchAPK {

    private const val LOWERALPHA = "abcdefghijklmnopqrstuvwxyz"
    private val UPPERALPHA = LOWERALPHA.toUpperCase()
    private val ALPHA = LOWERALPHA + UPPERALPHA
    private const val DIGITS = "0123456789"
    private val ALPHANUM = ALPHA + DIGITS
    private val ALPHANUMDOTS = "$ALPHANUM............"

    private fun genPackageName(prefix: String, length: Int): String {
        val builder = StringBuilder(length)
        builder.append(prefix)
        val len = length - prefix.length
        val random = SecureRandom()
        var next: Char
        var prev = prefix[prefix.length - 1]
        for (i in 0 until len) {
            next = if (prev == '.' || i == len - 1) {
                ALPHA[random.nextInt(ALPHA.length)]
            } else {
                ALPHANUMDOTS[random.nextInt(ALPHANUMDOTS.length)]
            }
            builder.append(next)
            prev = next
        }
        return builder.toString()
    }

    private fun findAndPatch(xml: ByteArray, from: String, to: String): Boolean {
        if (to.length > from.length)
            return false
        val buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asCharBuffer()
        val offList = mutableListOf<Int>()
        var i = 0
        loop@ while (i < buf.length - from.length) {
            for (j in from.indices) {
                if (buf.get(i + j) != from[j]) {
                    ++i
                    continue@loop
                }
            }
            offList.add(i)
            i += from.length
        }
        if (offList.isEmpty())
            return false

        val toBuf = to.toCharArray().copyOf(from.length)
        for (off in offList) {
            buf.position(off)
            buf.put(toBuf)
        }
        return true
    }

    private fun patchAndHide(context: Context, label: String): Boolean {
        // If not running as stub, and we are compatible with stub, use stub
        val src = if (!isRunningAsStub && SDK_INT >= 28 && Info.env.connectionMode == 3) {
            val stub = File(context.cacheDir, "stub.apk")
            val svc = get<GithubRawServices>()
            runCatching {
                svc.fetchFile(Info.remote.stub.link).blockingGet().byteStream().use {
                    it.writeTo(stub)
                }
            }.onFailure {
                Timber.e(it)
                return false
            }
            stub.path
        } else {
            context.packageCodePath
        }

        // Generate a new random package name and signature
        val repack = File(context.cacheDir, "patched.apk")
        val pkg = genPackageName("com.", BuildConfig.APPLICATION_ID.length)
        Config.keyStoreRaw = ""

        if (!patch(src, repack.path, pkg, label))
            return false

        // Install the application
        repack.setReadable(true, false)
        if (!Shell.su("force_pm_install $repack").exec().isSuccess)
            return false

        Config.suManager = pkg
        Config.export()
        Shell.su("pm uninstall ${BuildConfig.APPLICATION_ID}").submit()

        return true
    }

    @JvmStatic
    @JvmOverloads
    fun patch(apk: String, out: String, pkg: String, label: String = "Manager"): Boolean {
        try {
            val jar = JarMap.open(apk)
            val je = jar.getJarEntry(Const.ANDROID_MANIFEST)
            val xml = jar.getRawData(je)

            if (!findAndPatch(xml, BuildConfig.APPLICATION_ID, pkg) ||
                    !findAndPatch(xml, "Magisk Manager", label))
                return false

            // Write apk changes
            jar.getOutputStream(je).write(xml)
            val keys = Keygen()
            SignAPK.sign(keys.cert, keys.key, jar, FileOutputStream(out).buffered())
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        return true
    }

    fun patch(apk: File, out: File, pkg: String, label: String): Boolean {
        try {
            if (apk.length() < 1 shl 18) {
                // APK is smaller than 256K, must be stub
                return patch(apk.path, out.path, pkg, label)
            }

            // Try using the new APK to patch itself
            val loader = DynamicClassLoader(apk)
            val cls = loader.loadClass("a.a")

            for (m in cls.declaredMethods) {
                val pars = m.parameterTypes
                if (pars.size == 4 && pars[0] == String::class.java) {
                    return m.invoke(null, apk.path, out.path, pkg, label) as Boolean
                }
            }
            throw Exception("No matching method found")
        } catch (e: Exception) {
            Timber.e(e)
            // Fallback to use the current implementation
            return patch(apk.path, out.path, pkg, label)
        }
    }

    fun hideManager(context: Context, label: String) {
        Completable.fromAction {
            val progress = Notifications.progress(context, context.getString(R.string.hide_manager_title))
            Notifications.mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build())
            if (!patchAndHide(context, label))
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG)
            Notifications.mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID)
        }.subscribeK()
    }
}
