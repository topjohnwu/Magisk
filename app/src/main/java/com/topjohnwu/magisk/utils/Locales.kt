@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.utils

import android.annotation.SuppressLint
import android.content.res.Configuration
import android.content.res.Resources
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ResourceMgr
import com.topjohnwu.magisk.extensions.langTagToLocale
import com.topjohnwu.magisk.extensions.toLangTag
import io.reactivex.Single
import java.util.*
import kotlin.Comparator
import kotlin.collections.ArrayList

var currentLocale: Locale = Locale.getDefault()

@SuppressLint("ConstantLocale")
val defaultLocale: Locale = Locale.getDefault()

val availableLocales = Single.fromCallable {
    val compareId = R.string.app_changelog
    val config = ResourceMgr.resource.configuration
    val metrics = ResourceMgr.resource.displayMetrics
    val res = Resources(ResourceMgr.resource.assets, metrics, config)

    val locales = mutableListOf<Locale>().apply {
        // Add default locale
        add(Locale.ENGLISH)

        // Add some special locales
        add(Locale.TAIWAN)
        add(Locale("pt", "BR"))

        // Other locales
        val otherLocales = ResourceMgr.resource.assets.locales
            .map { it.langTagToLocale() }
            .distinctBy {
                config.setLocale(it)
                res.updateConfiguration(config, metrics)
                res.getString(compareId)
            }

        addAll(otherLocales)
    }.sortedWith(Comparator { a, b ->
        a.getDisplayName(a).toLowerCase(a)
            .compareTo(b.getDisplayName(b).toLowerCase(b))
    })

    config.setLocale(defaultLocale)
    res.updateConfiguration(config, metrics)
    val defName = res.getString(R.string.system_default)

    // Restore back to current locale
    config.setLocale(currentLocale)
    res.updateConfiguration(config, metrics)

    Pair(locales, defName)
}.map { (locales, defName) ->
    val names = ArrayList<String>(locales.size + 1)
    val values = ArrayList<String>(locales.size + 1)

    names.add(defName)
    values.add("")

    locales.forEach { locale ->
        names.add(locale.getDisplayName(locale))
        values.add(locale.toLangTag())
    }

    Pair(names.toTypedArray(), values.toTypedArray())
}.cache()!!

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
    ResourceMgr.resource.updateConfig()
}
