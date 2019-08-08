package com.topjohnwu.magisk.utils

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.net.Uri
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.rawResource
import com.topjohnwu.magisk.extensions.toShellCmd
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.io.SuFile
import java.util.*
import java.lang.reflect.Array as RArray

fun Intent.toCommand(args: MutableList<String>) {
    if (action != null) {
        args.add("-a")
        args.add(action!!)
    }
    if (component != null) {
        args.add("-n")
        args.add(component!!.flattenToString())
    }
    if (data != null) {
        args.add("-d")
        args.add(dataString!!)
    }
    if (categories != null) {
        for (cat in categories) {
            args.add("-c")
            args.add(cat)
        }
    }
    if (type != null) {
        args.add("-t")
        args.add(type!!)
    }
    val extras = extras
    if (extras != null) {
        loop@ for (key in extras.keySet()) {
            val v = extras.get(key) ?: continue
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
                v is ArrayList<*> -> {
                    if (v.size <= 0)
                    /* Impossible to know the type due to type erasure */
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
                    val len = RArray.getLength(v)
                    for (i in 0 until len) {
                        sb.append(RArray.get(v, i)!!.toString().replace(",", "\\,"))
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
}

fun startActivity(intent: Intent) {
    if (intent.component == null)
        return
    val args = ArrayList<String>()
    args.add("am")
    args.add("start")
    intent.toCommand(args)
    Shell.su(args.toShellCmd()).exec()
}

class RootUtils : Shell.Initializer() {

    override fun onInit(context: Context?, shell: Shell): Boolean {
        context ?: return false

        val job = shell.newJob()
        if (shell.isRoot) {
            job.add(context.rawResource(R.raw.util_functions))
                    .add(context.rawResource(R.raw.utils))
            Const.MAGISK_DISABLE_FILE = SuFile("/cache/.disable_magisk")
            Info.loadMagiskInfo()
        } else {
            job.add(context.rawResource(R.raw.nonroot_utils))
        }

        job.add("mount_partitions",
                "get_flags",
                "run_migrations",
                "export BOOTMODE=true")
                .exec()

        Info.keepVerity = ShellUtils.fastCmd("echo \$KEEPVERITY").toBoolean()
        Info.keepEnc = ShellUtils.fastCmd("echo \$KEEPFORCEENCRYPT").toBoolean()
        Info.recovery = ShellUtils.fastCmd("echo \$RECOVERYMODE").toBoolean()
        return true
    }

    companion object {

        fun rmAndLaunch(rm: String, component: ComponentName) {
            Shell.su("(rm_launch $rm ${component.flattenToString()})").exec()
        }
    }
}
