@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.utils

import android.annotation.SuppressLint
import android.content.Context
import android.content.ContextWrapper
import android.content.res.AssetManager
import android.content.res.Configuration
import android.content.res.Resources
import androidx.annotation.StringRes
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.langTagToLocale
import io.reactivex.Single
import java.util.*

var isRunningAsStub = false

var currentLocale: Locale = Locale.getDefault()
    private set

@SuppressLint("ConstantLocale")
val defaultLocale: Locale = Locale.getDefault()

val availableLocales = Single.fromCallable {
    val compareId = R.string.app_changelog
    mutableListOf<Locale>().apply {
        // Add default locale
        add(Locale.ENGLISH)

        // Add some special locales
        add(Locale.TAIWAN)
        add(Locale("pt", "BR"))

        val config = Configuration()
        val metrics = ResourceMgr.resource.displayMetrics
        val res = Resources(ResourceMgr.resource.assets, metrics, config)

        // Other locales
        val otherLocales = ResourceMgr.resource.assets.locales
                .map { it.langTagToLocale() }
                .distinctBy {
                    config.setLocale(it)
                    res.updateConfiguration(config, metrics)
                    res.getString(compareId)
                }

        listOf("", "").toTypedArray()

        addAll(otherLocales)
    }.sortedWith(Comparator { a, b ->
        a.getDisplayName(a).toLowerCase(a)
                .compareTo(b.getDisplayName(b).toLowerCase(b))
    })
}.cache()!!

private val addAssetPath by lazy {
    AssetManager::class.java.getMethod("addAssetPath", String::class.java)
}

fun AssetManager.addAssetPath(path: String) {
    addAssetPath.invoke(this, path)
}

fun Context.wrap(global: Boolean = true): Context
        = if (!global) ResourceMgr.ResContext(this) else ResourceMgr.GlobalResContext(this)

object ResourceMgr {

    lateinit var resource: Resources
    private lateinit var resApk: String

    fun init(context: Context) {
        resource = context.resources
        if (isRunningAsStub)
            resApk = DynAPK.current(context).path
    }

    // Override locale and inject resources from dynamic APK
    private fun Resources.patch(config: Configuration = Configuration(configuration)): Resources {
        config.setLocale(currentLocale)
        updateConfiguration(config, displayMetrics)
        if (isRunningAsStub)
            assets.addAssetPath(resApk)
        return this
    }

    fun reload(config: Configuration = Configuration(resource.configuration)) {
        val localeConfig = Config.locale
        currentLocale = when {
            localeConfig.isEmpty() -> defaultLocale
            else -> localeConfig.langTagToLocale()
        }
        Locale.setDefault(currentLocale)
        resource.patch(config)
    }

    fun getString(locale: Locale, @StringRes id: Int): String {
        val config = Configuration()
        config.setLocale(locale)
        return Resources(resource.assets, resource.displayMetrics, config).getString(id)
    }

    open class GlobalResContext(base: Context) : ContextWrapper(base) {
        open val mRes: Resources get() = resource
        private val loader by lazy { javaClass.classLoader!! }

        override fun getResources(): Resources {
            return mRes
        }

        override fun getClassLoader(): ClassLoader {
            return loader
        }

        override fun createConfigurationContext(config: Configuration): Context {
            return ResContext(super.createConfigurationContext(config))
        }
    }

    class ResContext(base: Context) : GlobalResContext(base) {
        override val mRes by lazy { base.resources.patch() }
    }

}
