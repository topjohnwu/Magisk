package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.ComponentInfo
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.content.pm.PackageManager.*
import android.net.Uri
import android.provider.OpenableColumns
import com.topjohnwu.magisk.App
import java.io.File
import java.io.FileNotFoundException

val packageName: String
    get() {
        val app: App by inject()
        return app.packageName
    }

val PackageInfo.processes
    get() = activities?.processNames.orEmpty() +
            services?.processNames.orEmpty() +
            receivers?.processNames.orEmpty() +
            providers?.processNames.orEmpty()

val Array<out ComponentInfo>.processNames get() = mapNotNull { it.processName }

val ApplicationInfo.packageInfo: PackageInfo?
    get() {
        val pm: PackageManager by inject()

        return try {
            val request = GET_ACTIVITIES or
                    GET_SERVICES or
                    GET_RECEIVERS or
                    GET_PROVIDERS
            pm.getPackageInfo(packageName, request)
        } catch (e1: Exception) {
            try {
                pm.activities(packageName).apply {
                    services = pm.services(packageName)
                    receivers = pm.receivers(packageName)
                    providers = pm.providers(packageName)
                }
            } catch (e2: Exception) {
                null
            }
        }
    }

val Uri.fileName: String get() {
    var name: String? = null
    App.self.contentResolver.query(this, null, null, null, null)?.use { c ->
        val nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        if (nameIndex != -1) {
            c.moveToFirst()
            name = c.getString(nameIndex)
        }
    }
    if (name == null && path != null) {
        val idx = path!!.lastIndexOf('/')
        name = path!!.substring(idx + 1)
    }
    return name.orEmpty()
}

fun PackageManager.activities(packageName: String) =
    getPackageInfo(packageName, GET_ACTIVITIES)

fun PackageManager.services(packageName: String) =
    getPackageInfo(packageName, GET_SERVICES).services

fun PackageManager.receivers(packageName: String) =
    getPackageInfo(packageName, GET_RECEIVERS).receivers

fun PackageManager.providers(packageName: String) =
    getPackageInfo(packageName, GET_PROVIDERS).providers

fun Context.rawResource(id: Int) = resources.openRawResource(id)

fun Context.readUri(uri: Uri) = contentResolver.openInputStream(uri) ?: throw FileNotFoundException()

fun ApplicationInfo.findAppLabel(pm: PackageManager): String {
    return pm.getApplicationLabel(this)?.toString().orEmpty()
}

fun Intent.startActivity(context: Context) = context.startActivity(this)

fun File.provide(): Uri {
    val context: Context by inject()
    return FileProvider.getUriForFile(context, "com.topjohnwu.magisk.fileprovider", this)
}

fun File.mv(destination: File) {
    inputStream().copyTo(destination)
    deleteRecursively()
}

fun String.toFile() = File(this)

fun Intent.chooser(title: String = "Pick an app") = Intent.createChooser(this, title)