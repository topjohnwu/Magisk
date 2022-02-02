@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core

import android.app.Activity
import android.content.ComponentName
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.res.AssetManager
import android.content.res.Configuration
import android.content.res.Resources
import android.util.DisplayMetrics
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.utils.syncLocale
import com.topjohnwu.magisk.di.AppContext

lateinit var AppApkPath: String

fun AssetManager.addAssetPath(path: String) = StubApk.addAssetPath(this, path)

fun Context.wrap(): Context = if (this is PatchedContext) this else PatchedContext(this)

private class PatchedContext(base: Context) : ContextWrapper(base) {
    init { base.resources.patch() }
    override fun getClassLoader() = javaClass.classLoader!!
    override fun createConfigurationContext(config: Configuration) =
        super.createConfigurationContext(config).wrap()
}

fun Resources.patch(): Resources {
    syncLocale()
    if (isRunningAsStub)
        assets.addAssetPath(AppApkPath)
    return this
}

fun createNewResources(): Resources {
    val asset = AssetManager::class.java.newInstance()
    asset.addAssetPath(AppApkPath)
    val config = Configuration(AppContext.resources.configuration)
    val metrics = DisplayMetrics()
    metrics.setTo(AppContext.resources.displayMetrics)
    return Resources(asset, metrics, config)
}

fun Class<*>.cmp(pkg: String) =
    ComponentName(pkg, Info.stub?.classToComponent?.get(name) ?: name)

inline fun <reified T> Activity.redirect() = Intent(intent)
    .setComponent(T::class.java.cmp(packageName))
    .setFlags(0)

inline fun <reified T> Context.intent() = Intent().setComponent(T::class.java.cmp(packageName))

// Keep a reference to these resources to prevent it from
// being removed when running "remove unused resources"
val shouldKeepResources = listOf(
    R.string.no_info_provided,
    R.string.release_notes,
    R.string.invalid_update_channel,
    R.string.update_available,
    R.drawable.ic_device,
    R.drawable.ic_more,
    R.drawable.ic_magisk_delete,
    R.drawable.ic_refresh_data_md2,
    R.drawable.ic_order_date,
    R.drawable.ic_order_name,
    R.array.allow_timeout,
)
