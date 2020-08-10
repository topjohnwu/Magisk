@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core.utils

import android.annotation.SuppressLint
import android.content.res.AssetManager
import android.content.res.Configuration
import android.content.res.Resources
import android.util.DisplayMetrics
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.ResMgr
import com.topjohnwu.magisk.core.addAssetPath
import com.topjohnwu.magisk.ktx.langTagToLocale
import com.topjohnwu.magisk.ktx.toLangTag
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.*
import kotlin.Comparator
import kotlin.collections.ArrayList

var currentLocale: Locale = Locale.getDefault()

@SuppressLint("ConstantLocale")
val defaultLocale: Locale = Locale.getDefault()

private var cachedLocales: Pair<Array<String>, Array<String>>? = null

suspend fun availableLocales() = cachedLocales ?:
withContext(Dispatchers.Default) {
    val compareId = R.string.app_changelog

    // Create a completely new resource to prevent cross talk over app's configs
    val asset = AssetManager::class.java.newInstance().apply { addAssetPath(ResMgr.apk) }
    val config = Configuration(ResMgr.resource.configuration)
    val metrics = DisplayMetrics().apply { setTo(ResMgr.resource.displayMetrics) }
    val res = Resources(asset, metrics, config)

    val locales = ArrayList<String>().apply {
        // Add default locale
        add("en")

        // Add some special locales
        add("zh-TW")
        add("pt-BR")

        // Then add all supported locales
        addAll(res.assets.locales)
    }.map {
        it.langTagToLocale()
    }.distinctBy {
        config.setLocale(it)
        res.updateConfiguration(config, metrics)
        res.getString(compareId)
    }.sortedWith(Comparator { a, b ->
        a.getDisplayName(a).compareTo(b.getDisplayName(b), true)
    })

    config.setLocale(defaultLocale)
    res.updateConfiguration(config, metrics)
    val defName = res.getString(R.string.system_default)

    val names = ArrayList<String>(locales.size + 1)
    val values = ArrayList<String>(locales.size + 1)

    names.add(defName)
    values.add("")

    locales.forEach { locale ->
        names.add(locale.getDisplayName(locale))
        values.add(locale.toLangTag())
    }

    (names.toTypedArray() to values.toTypedArray()).also { cachedLocales = it }
}

fun Resources.updateConfig(config: Configuration = configuration) {
    config.setLocale(currentLocale)
    updateConfiguration(config, displayMetrics)
}

fun refreshLocale() {
    val localeConfig = Config.locale
    currentLocale = when {
        localeConfig.isEmpty() -> defaultLocale
        else -> localeConfig.langTagToLocale()
    }
    Locale.setDefault(currentLocale)
    ResMgr.resource.updateConfig()
}
