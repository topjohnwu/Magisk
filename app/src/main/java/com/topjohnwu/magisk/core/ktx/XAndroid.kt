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
import android.net.Uri
import android.os.Build
import android.os.Build.VERSION.SDK_INT
import android.os.Process
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.Toast
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.getSystemService
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import java.io.File
import kotlin.Array
import kotlin.String
import java.lang.reflect.Array as JArray

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

fun Intent.startActivityWithRoot() {
    val args = mutableListOf("am", "start", "--user", Const.USER_ID.toString())
    val cmd = toCommand(args).joinToString(" ")
    Shell.cmd(cmd).submit()
}

fun Intent.toCommand(args: MutableList<String> = mutableListOf()): MutableList<String> {
    action?.also {
        args.add("-a")
        args.add(it)
    }
    component?.also {
        args.add("-n")
        args.add(it.flattenToString())
    }
    data?.also {
        args.add("-d")
        args.add(it.toString())
    }
    categories?.also {
        for (cat in it) {
            args.add("-c")
            args.add(cat)
        }
    }
    type?.also {
        args.add("-t")
        args.add(it)
    }
    extras?.also {
        loop@ for (key in it.keySet()) {
            val v = it[key] ?: continue
            var value: Any = v
            val arg: String
            when {
                v is String -> arg = "--es"
                v is Boolean -> arg = "--ez"
                v is Int -> arg = "--ei"
                v is Long -> arg = "--el"
                v is Float -> arg = "--ef"
                v is Uri -> arg = "--eu"
                v is ComponentName -> {
                    arg = "--ecn"
                    value = v.flattenToString()
                }
                v is List<*> -> {
                    if (v.isEmpty())
                        continue@loop

                    arg = if (v[0] is Int)
                        "--eial"
                    else if (v[0] is Long)
                        "--elal"
                    else if (v[0] is Float)
                        "--efal"
                    else if (v[0] is String)
                        "--esal"
                    else
                        continue@loop  /* Unsupported */

                    val sb = StringBuilder()
                    for (o in v) {
                        sb.append(o.toString().replace(",", "\\,"))
                        sb.append(',')
                    }
                    // Remove trailing comma
                    sb.deleteCharAt(sb.length - 1)
                    value = sb
                }
                v.javaClass.isArray -> {
                    arg = if (v is IntArray)
                        "--eia"
                    else if (v is LongArray)
                        "--ela"
                    else if (v is FloatArray)
                        "--efa"
                    else if (v is Array<*> && v.isArrayOf<String>())
                        "--esa"
                    else
                        continue@loop  /* Unsupported */

                    val sb = StringBuilder()
                    val len = JArray.getLength(v)
                    for (i in 0 until len) {
                        sb.append(JArray.get(v, i)!!.toString().replace(",", "\\,"))
                        sb.append(',')
                    }
                    // Remove trailing comma
                    sb.deleteCharAt(sb.length - 1)
                    value = sb
                }
                else -> continue@loop
            }  /* Unsupported */

            args.add(arg)
            args.add(key)
            args.add(value.toString())
        }
    }
    args.add("-f")
    args.add(flags.toString())
    return args
}

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
