@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core.utils

import android.annotation.SuppressLint
import android.content.res.Configuration
import android.content.res.Resources
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ActivityTracker
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.createNewResources
import com.topjohnwu.magisk.core.di.AppContext
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.*

var currentLocale: Locale = Locale.getDefault()

@SuppressLint("ConstantLocale")
val defaultLocale: Locale = Locale.getDefault()

private var cachedLocales: Pair<Array<String>, Array<String>>? = null

suspend fun availableLocales() = cachedLocales ?:
withContext(Dispatchers.Default) {
    val compareId = R.string.app_changelog

    // Create a completely new resource to prevent cross talk over active configs
    val res = createNewResources()

    fun changeLocale(locale: Locale) {
        res.configuration.setLocale(locale)
        res.updateConfiguration(res.configuration, res.displayMetrics)
    }

    val locales = ArrayList<String>().apply {
        // Add default locale
        add("en")

        // Add some special locales
        add("zh-TW")
        add("pt-BR")

        // Then add all supported locales
        addAll(Resources.getSystem().assets.locales)
    }.map {
        Locale.forLanguageTag(it)
    }.distinctBy {
        changeLocale(it)
        res.getString(compareId)
    }.sortedWith { a, b ->
        a.getDisplayName(a).compareTo(b.getDisplayName(b), true)
    }

    changeLocale(defaultLocale)
    val defName = res.getString(R.string.system_default)

    val names = ArrayList<String>(locales.size + 1)
    val values = ArrayList<String>(locales.size + 1)

    names.add(defName)
    values.add("")

    locales.forEach { locale ->
        names.add(locale.getDisplayName(locale))
        values.add(locale.toLanguageTag())
    }

    (names.toTypedArray() to values.toTypedArray()).also { cachedLocales = it }
}

fun Resources.setConfig(config: Configuration) {
    config.setLocale(currentLocale)
    updateConfiguration(config, displayMetrics)
}

fun Resources.syncLocale() = setConfig(configuration)

fun refreshLocale() {
    val localeConfig = Config.locale
    currentLocale = when {
        localeConfig.isEmpty() -> defaultLocale
        else -> Locale.forLanguageTag(localeConfig)
    }
    Locale.setDefault(currentLocale)
    AppContext.resources.syncLocale()
    ActivityTracker.foreground?.recreate()
}
