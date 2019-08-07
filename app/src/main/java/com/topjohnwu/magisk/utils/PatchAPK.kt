package com.topjohnwu.magisk.utils

import android.content.ComponentName
import android.content.Context
import android.widget.Toast
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.ui.SplashActivity
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
        if (from.length != to.length)
            return false
        val buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asCharBuffer()
        val offList = mutableListOf<Int>()
        var i = 0
        while (i < buf.length - from.length) {
            var match = true
            for (j in 0 until from.length) {
                if (buf.get(i + j) != from[j]) {
                    match = false
                    break
                }
            }
            if (match) {
                offList.add(i)
                i += from.length
            }
            ++i
        }
        if (offList.isEmpty())
            return false
        for (off in offList) {
            buf.position(off)
            buf.put(to)
        }
        return true
    }

    private fun findAndPatch(xml: ByteArray, a: Int, b: Int): Boolean {
        val buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer()
        val len = xml.size / 4
        for (i in 0 until len) {
            if (buf.get(i) == a) {
                buf.put(i, b)
                return true
            }
        }
        return false
    }

    private fun patchAndHide(context: Context): Boolean {
        // Generate a new app with random package name
        val repack = File(context.filesDir, "patched.apk")
        val pkg = genPackageName("com.", BuildConfig.APPLICATION_ID.length)

        if (!patch(context.packageCodePath, repack.path, pkg))
            return false

        // Install the application
        repack.setReadable(true, false)
        if (!Shell.su("pm install $repack").exec().isSuccess)
            return false

        Config.suManager = pkg
        Config.export()
        RootUtils.rmAndLaunch(BuildConfig.APPLICATION_ID,
                ComponentName(pkg, ClassMap.get<Class<*>>(SplashActivity::class.java).name))

        return true
    }

    @JvmStatic
    fun patch(apk: String, out: String, pkg: String): Boolean {
        try {
            val jar = JarMap(apk)
            val je = jar.getJarEntry(Const.ANDROID_MANIFEST)
            val xml = jar.getRawData(je)

            if (!findAndPatch(xml, BuildConfig.APPLICATION_ID, pkg) ||
                    !findAndPatch(xml, R.string.app_name, R.string.re_app_name))
                return false

            // Write apk changes
            jar.getOutputStream(je).write(xml)
            SignAPK.sign(jar, FileOutputStream(out).buffered())
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        return true
    }

    fun hideManager(context: Context) {
        Completable.fromAction {
            val progress = Notifications.progress(context, context.getString(R.string.hide_manager_title))
            Notifications.mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build())
            if (!patchAndHide(context))
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG)
            Notifications.mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID)
        }.subscribeK()
    }
}
