package com.topjohnwu.magisk.ktx

import android.annotation.SuppressLint
import android.app.Activity
import android.content.ComponentName
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.content.pm.PackageManager.*
import android.content.res.Configuration
import android.content.res.Resources
import android.database.Cursor
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.drawable.AdaptiveIconDrawable
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.LayerDrawable
import android.net.Uri
import android.os.Build.VERSION.SDK_INT
import android.text.PrecomputedText
import android.view.View
import android.view.ViewGroup
import android.view.ViewTreeObserver
import android.view.inputmethod.InputMethodManager
import android.widget.TextView
import androidx.annotation.ColorRes
import androidx.annotation.DrawableRes
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.core.content.getSystemService
import androidx.core.net.toUri
import androidx.core.text.PrecomputedTextCompat
import androidx.core.view.isGone
import androidx.core.widget.TextViewCompat
import androidx.databinding.BindingAdapter
import androidx.fragment.app.Fragment
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import androidx.lifecycle.lifecycleScope
import androidx.transition.AutoTransition
import androidx.transition.TransitionManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.utils.DynamicClassLoader
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.File
import java.lang.reflect.Array as JArray

fun Context.rawResource(id: Int) = resources.openRawResource(id)

fun Context.getBitmap(id: Int): Bitmap {
    var drawable = AppCompatResources.getDrawable(this, id)!!
    if (drawable is BitmapDrawable)
        return drawable.bitmap
    if (SDK_INT >= 26 && drawable is AdaptiveIconDrawable) {
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
    if (SDK_INT >= 24) {
        createDeviceProtectedStorageContext()
    } else { this }

fun Intent.startActivity(context: Context) = context.startActivity(this)

fun Intent.startActivityWithRoot() {
    val args = mutableListOf("am", "start", "--user", Const.USER_ID.toString())
    val cmd = toCommand(args).joinToString(" ")
    Shell.su(cmd).submit()
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

fun Intent.chooser(title: String = "Pick an app") = Intent.createChooser(this, title)

fun Context.cachedFile(name: String) = File(cacheDir, name)

fun <Result> Cursor.toList(transformer: (Cursor) -> Result): List<Result> {
    val out = mutableListOf<Result>()
    while (moveToNext()) out.add(transformer(this))
    return out
}

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

fun Intent.exists(packageManager: PackageManager) = resolveActivity(packageManager) != null

fun Context.colorCompat(@ColorRes id: Int) = try {
    ContextCompat.getColor(this, id)
} catch (e: Resources.NotFoundException) {
    null
}

fun Context.colorStateListCompat(@ColorRes id: Int) = try {
    ContextCompat.getColorStateList(this, id)
} catch (e: Resources.NotFoundException) {
    null
}

fun Context.drawableCompat(@DrawableRes id: Int) = AppCompatResources.getDrawable(this, id)
/**
 * Pass [start] and [end] dimensions, function will return left and right
 * with respect to RTL layout direction
 */
fun Context.startEndToLeftRight(start: Int, end: Int): Pair<Int, Int> {
    if (resources.configuration.layoutDirection == View.LAYOUT_DIRECTION_RTL) {
        return end to start
    }
    return start to end
}

fun Context.openUrl(url: String) = Utils.openLink(this, url.toUri())

inline fun <reified T> T.createClassLoader(apk: File) =
    DynamicClassLoader(apk, T::class.java.classLoader)

fun Context.unwrap(): Context {
    var context = this
    while (true) {
        if (context is ContextWrapper)
            context = context.baseContext
        else
            break
    }
    return context
}

fun Context.hasPermissions(vararg permissions: String) = permissions.all {
    ContextCompat.checkSelfPermission(this, it) == PERMISSION_GRANTED
}

fun Activity.hideKeyboard() {
    val view = currentFocus ?: return
    getSystemService<InputMethodManager>()
        ?.hideSoftInputFromWindow(view.windowToken, 0)
    view.clearFocus()
}

fun Fragment.hideKeyboard() {
    activity?.hideKeyboard()
}

fun View.setOnViewReadyListener(callback: () -> Unit) = addOnGlobalLayoutListener(true, callback)

fun View.addOnGlobalLayoutListener(oneShot: Boolean = false, callback: () -> Unit) =
    viewTreeObserver.addOnGlobalLayoutListener(object :
        ViewTreeObserver.OnGlobalLayoutListener {
        override fun onGlobalLayout() {
            if (oneShot) viewTreeObserver.removeOnGlobalLayoutListener(this)
            callback()
        }
    })

fun ViewGroup.startAnimations() {
    val transition = AutoTransition()
        .setInterpolator(FastOutSlowInInterpolator())
        .setDuration(400)
        .excludeTarget(R.id.main_toolbar, true)
    TransitionManager.beginDelayedTransition(
        this,
        transition
    )
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
