package com.topjohnwu.magisk.core.ktx

import android.annotation.SuppressLint
import android.app.Activity
import android.content.*
import android.content.pm.ApplicationInfo
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.drawable.AdaptiveIconDrawable
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.LayerDrawable
import android.os.Build
import android.os.Build.VERSION.SDK_INT
import android.os.Process
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.Toast
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.getSystemService
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.superuser.internal.UiThreadHandler
import java.io.File
import kotlin.String

fun Context.rawResource(id: Int) = resources.openRawResource(id)

fun Context.getBitmap(id: Int): Bitmap {
    var drawable = AppCompatResources.getDrawable(this, id)!!
    if (drawable is BitmapDrawable)
        return drawable.bitmap
    if (SDK_INT >= Build.VERSION_CODES.O && drawable is AdaptiveIconDrawable) {
        drawable = LayerDrawable(arrayOf(drawable.background, drawable.foreground))
    }
    val bitmap = Bitmap.createBitmap(
        drawable.intrinsicWidth, drawable.intrinsicHeight,
        Bitmap.Config.ARGB_8888
    )
    val canvas = Canvas(bitmap)
    drawable.setBounds(0, 0, canvas.width, canvas.height)
    drawable.draw(canvas)
    return bitmap
}

val Context.deviceProtectedContext: Context get() =
    if (SDK_INT >= Build.VERSION_CODES.N) {
        createDeviceProtectedStorageContext()
    } else { this }

fun Context.cachedFile(name: String) = File(cacheDir, name)

fun ApplicationInfo.getLabel(pm: PackageManager): String {
    runCatching {
        if (labelRes > 0) {
            val res = pm.getResourcesForApplication(this)
            val config = Configuration()
            config.setLocale(currentLocale)
            res.updateConfiguration(config, res.displayMetrics)
            return res.getString(labelRes)
        }
    }

    return loadLabel(pm).toString()
}

fun Context.unwrap(): Context {
    var context = this
    while (context is ContextWrapper)
        context = context.baseContext
    return context
}

fun Activity.hideKeyboard() {
    val view = currentFocus ?: return
    getSystemService<InputMethodManager>()
        ?.hideSoftInputFromWindow(view.windowToken, 0)
    view.clearFocus()
}

val View.activity: Activity get() {
    var context = context
    while(true) {
        if (context !is ContextWrapper)
            error("View is not attached to activity")
        if (context is Activity)
            return context
        context = context.baseContext
    }
}

@SuppressLint("PrivateApi")
fun getProperty(key: String, def: String): String {
    runCatching {
        val clazz = Class.forName("android.os.SystemProperties")
        val get = clazz.getMethod("get", String::class.java, String::class.java)
        return get.invoke(clazz, key, def) as String
    }
    return def
}

@SuppressLint("InlinedApi")
@Throws(PackageManager.NameNotFoundException::class)
fun PackageManager.getPackageInfo(uid: Int, pid: Int): PackageInfo? {
    val flag = PackageManager.MATCH_UNINSTALLED_PACKAGES
    val pkgs = getPackagesForUid(uid) ?: throw PackageManager.NameNotFoundException()
    if (pkgs.size > 1) {
        if (pid <= 0) {
            return null
        }
        // Try to find package name from PID
        val proc = RootUtils.obj?.getAppProcess(pid)
        if (proc == null) {
            if (uid == Process.SHELL_UID) {
                // It is possible that some apps installed are sharing UID with shell.
                // We will not be able to find a package from the active process list,
                // because the client is forked from ADB shell, not any app process.
                return getPackageInfo("com.android.shell", flag)
            }
        } else if (uid == proc.uid) {
            return getPackageInfo(proc.pkgList[0], flag)
        }

        return null
    }
    if (pkgs.size == 1) {
        return getPackageInfo(pkgs[0], flag)
    }
    throw PackageManager.NameNotFoundException()
}

fun Context.registerRuntimeReceiver(receiver: BroadcastReceiver, filter: IntentFilter) {
    APKInstall.registerReceiver(this, receiver, filter)
}

fun Context.selfLaunchIntent(): Intent {
    val pm = packageManager
    val intent = pm.getLaunchIntentForPackage(packageName)!!
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
    return intent
}

fun Context.toast(msg: CharSequence, duration: Int) {
    UiThreadHandler.run { Toast.makeText(this, msg, duration).show() }
}

fun Context.toast(resId: Int, duration: Int) {
    UiThreadHandler.run { Toast.makeText(this, resId, duration).show() }
}
