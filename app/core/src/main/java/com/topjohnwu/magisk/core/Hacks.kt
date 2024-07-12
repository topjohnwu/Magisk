@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core

import android.content.ComponentName
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.res.Configuration
import android.content.res.Resources
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.ktx.unwrap
import com.topjohnwu.magisk.core.utils.LocaleSetting

fun Resources.addAssetPath(path: String) = StubApk.addAssetPath(this, path)

fun Resources.patch(): Resources {
    if (isRunningAsStub)
        addAssetPath(AppApkPath)
    LocaleSetting.instance.updateResource(this)
    return this
}

fun Context.patch(): Context {
    unwrap().resources.patch()
    return this
}

// Wrapping is only necessary for ContextThemeWrapper to support configuration overrides
fun Context.wrap(): Context {
    patch()
    return object : ContextWrapper(this) {
        override fun createConfigurationContext(config: Configuration): Context {
            return super.createConfigurationContext(config).wrap()
        }
    }
}

fun Class<*>.cmp(pkg: String) =
    ComponentName(pkg, Info.stub?.classToComponent?.get(name) ?: name)

inline fun <reified T> Context.intent() = Intent().setComponent(T::class.java.cmp(packageName))

// Keep a reference to these resources to prevent it from
// being removed when running "remove unused resources"
val shouldKeepResources = listOf(
    R.string.no_info_provided,
    R.string.release_notes,
    R.string.invalid_update_channel,
    R.string.update_available,
    R.string.app_changelog,
    R.string.home_item_source,
    R.drawable.ic_more,
    R.array.allow_timeout,
)
