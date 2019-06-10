package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.content.res.Resources
import android.net.Uri
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.work.*
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.OldModule
import com.topjohnwu.magisk.model.update.UpdateCheckService
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.io.SuFile
import java.util.*
import java.util.concurrent.TimeUnit

object Utils {

    val isCanary: Boolean
        get() = BuildConfig.VERSION_NAME.contains("-")

    fun toast(msg: CharSequence, duration: Int) {
        UiThreadHandler.run { Toast.makeText(get(), msg, duration).show() }
    }

    fun toast(resId: Int, duration: Int) {
        UiThreadHandler.run { Toast.makeText(get(), resId, duration).show() }
    }

    fun dlString(url: String): String {
        val s = Networking.get(url).execForString().result
        return s ?: ""
    }

    fun getPrefsInt(prefs: SharedPreferences, key: String, def: Int = 0): Int {
        return prefs.getString(key, def.toString())!!.toInt()
    }

    fun dpInPx(dp: Int): Int {
        val scale = get<Resources>().displayMetrics.density
        return (dp * scale + 0.5).toInt()
    }

    fun fmt(fmt: String, vararg args: Any): String {
        return String.format(Locale.US, fmt, *args)
    }

    fun getAppLabel(info: ApplicationInfo, pm: PackageManager): String {
        try {
            if (info.labelRes > 0) {
                val res = pm.getResourcesForApplication(info)
                val config = Configuration()
                config.setLocale(LocaleManager.locale)
                res.updateConfiguration(config, res.displayMetrics)
                return res.getString(info.labelRes)
            }
        } catch (ignored: Exception) {
        }

        return info.loadLabel(pm).toString()
    }

    fun getLegalFilename(filename: CharSequence): String {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("*", "").replace("/", "_")
                .replace("#", "").replace("@", "").replace("\\", "_")
    }

    @WorkerThread
    fun loadModulesLeanback(): Map<String, OldModule> {
        val moduleMap = ValueSortedMap<String, OldModule>()
        val path = SuFile(Const.MAGISK_PATH)
        val modules = path.listFiles { _, name -> name != "lost+found" && name != ".core" }
        for (file in modules!!) {
            if (file.isFile) continue
            val module = OldModule(Const.MAGISK_PATH + "/" + file.name)
            moduleMap[module.id] = module
        }
        return moduleMap
    }

    fun showSuperUser(): Boolean {
        return Shell.rootAccess() && (Const.USER_ID == 0
                || Config.suMultiuserMode != Config.Value.MULTIUSER_MODE_OWNER_MANAGED)
    }

    fun scheduleUpdateCheck() {
        if (Config.checkUpdate) {
            val constraints = Constraints.Builder()
                    .setRequiredNetworkType(NetworkType.CONNECTED)
                    .setRequiresDeviceIdle(true)
                    .build()
            val request = PeriodicWorkRequest
                    .Builder(ClassMap[UpdateCheckService::class.java], 12, TimeUnit.HOURS)
                    .setConstraints(constraints)
                    .build()
            WorkManager.getInstance().enqueueUniquePeriodicWork(
                    Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID,
                    ExistingPeriodicWorkPolicy.REPLACE, request)
        } else {
            WorkManager.getInstance().cancelUniqueWork(Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID)
        }
    }

    fun openLink(context: Context, link: Uri) {
        val intent = Intent(Intent.ACTION_VIEW, link)
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        if (intent.resolveActivity(context.packageManager) != null) {
            context.startActivity(intent)
        } else {
            toast(R.string.open_link_failed_toast, Toast.LENGTH_SHORT)
        }
    }

}
