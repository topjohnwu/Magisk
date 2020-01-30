package com.topjohnwu.magisk.core.utils

import android.content.Context
import android.os.Build.VERSION.SDK_INT
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.signing.JarMap
import com.topjohnwu.signing.SignAPK
import com.topjohnwu.superuser.Shell
import io.reactivex.Single
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.security.SecureRandom

object PatchAPK {

    private const val ALPHA = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    private const val DIGITS = "0123456789"
    const val ALPHANUM = ALPHA + DIGITS
    private const val ALPHANUMDOTS = "$ALPHANUM............"

    private const val APP_ID = "com.topjohnwu.magisk"
    private const val APP_NAME = "Magisk Manager"

    private fun genPackageName(prefix: String, length: Int): CharSequence {
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
        return builder
    }

    private fun findAndPatch(xml: ByteArray, from: CharSequence, to: CharSequence): Boolean {
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

        val toBuf = to.toString().toCharArray().copyOf(from.length)
        for (off in offList) {
            buf.position(off)
            buf.put(toBuf)
        }
        return true
    }

    fun patch(apk: String, out: String, pkg: CharSequence, label: CharSequence): Boolean {
        try {
            val jar = JarMap.open(apk)
            val je = jar.getJarEntry(Const.ANDROID_MANIFEST)
            val xml = jar.getRawData(je)

            if (!findAndPatch(xml, APP_ID, pkg) ||
                !findAndPatch(xml, APP_NAME, label))
                return false

            // Write apk changes
            jar.getOutputStream(je).write(xml)
            val keys = Keygen(get())
            SignAPK.sign(keys.cert, keys.key, jar, FileOutputStream(out).buffered())
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        return true
    }

    private fun patchAndHide(context: Context, label: String): Boolean {
        val dlStub = !isRunningAsStub && SDK_INT >= 28 &&
                Info.env.magiskVersionCode >= Const.Version.PROVIDER_CONNECT
        val src = if (dlStub) {
            val stub = File(context.cacheDir, "stub.apk")
            val svc = get<GithubRawServices>()
            try {
                svc.fetchFile(Info.remote.stub.link).blockingGet().byteStream().use {
                    it.writeTo(stub)
                }
            } catch (e: Exception) {
                Timber.e(e)
                return false
            }
            stub.path
        } else {
            context.packageCodePath
        }

        // Generate a new random package name and signature
        val repack = File(context.cacheDir, "patched.apk")
        val pkg = genPackageName("com.", APP_ID.length)
        Config.keyStoreRaw = ""

        if (!patch(src, repack.path, pkg, label))
            return false

        // Install the application
        repack.setReadable(true, false)
        if (!Shell.su("force_pm_install $repack").exec().isSuccess)
            return false

        Config.suManager = pkg.toString()
        Config.export()
        Shell.su("pm uninstall $APP_ID").submit()

        return true
    }

    fun hideManager(context: Context, label: String) {
        val progress = Notifications.progress(context, context.getString(R.string.hide_manager_title))
        Notifications.mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build())
        Single.fromCallable {
            patchAndHide(context, label)
        }.subscribeK {
            if (!it)
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG)
            Notifications.mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID)
        }
    }
}
