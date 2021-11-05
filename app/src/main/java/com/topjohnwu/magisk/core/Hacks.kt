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
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.utils.refreshLocale
import com.topjohnwu.magisk.core.utils.updateConfig

fun AssetManager.addAssetPath(path: String) {
    DynAPK.addAssetPath(this, path)
}

fun Context.wrap(inject: Boolean = false): Context =
    if (inject) ReInjectedContext(this) else InjectedContext(this)

fun Class<*>.cmp(pkg: String) =
    ComponentName(pkg, Info.stub?.classToComponent?.get(name) ?: name)

inline fun <reified T> Activity.redirect() = Intent(intent)
    .setComponent(T::class.java.cmp(packageName))
    .setFlags(0)

inline fun <reified T> Context.intent() = Intent().setComponent(T::class.java.cmp(packageName))

private open class InjectedContext(base: Context) : ContextWrapper(base) {
    open val res: Resources get() = AssetHack.resource
    override fun getAssets(): AssetManager = res.assets
    override fun getResources() = res
    override fun getClassLoader() = javaClass.classLoader!!
    override fun createConfigurationContext(config: Configuration): Context {
        return super.createConfigurationContext(config).wrap(true)
    }
}

private class ReInjectedContext(base: Context) : InjectedContext(base) {
    override val res by lazy { base.resources.patch() }
    private fun Resources.patch(): Resources {
        updateConfig()
        if (isRunningAsStub)
            assets.addAssetPath(AssetHack.apk)
        return this
    }
}

object AssetHack {

    lateinit var resource: Resources
    lateinit var apk: String

    fun init(context: Context) {
        resource = context.resources
        refreshLocale()
        if (isRunningAsStub) {
            apk = DynAPK.current(context).path
            resource.assets.addAssetPath(apk)
        } else {
            apk = context.packageResourcePath
        }
    }

    fun newResource(): Resources {
        val asset = AssetManager::class.java.newInstance()
        asset.addAssetPath(apk)
        val config = Configuration(resource.configuration)
        val metrics = DisplayMetrics()
        metrics.setTo(resource.displayMetrics)
        return Resources(asset, metrics, config)
    }
}

// Keep a reference to these resources to prevent it from
// being removed when running "remove unused resources"
val shouldKeepResources = listOf(
    R.string.no_info_provided,
    R.string.release_notes,
    R.string.invalid_update_channel,
    R.string.update_available,
    R.string.safetynet_api_error,
    R.drawable.ic_device,
    R.drawable.ic_hide_select_md2,
    R.drawable.ic_more,
    R.drawable.ic_magisk_delete
)
