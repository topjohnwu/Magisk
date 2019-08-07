package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.ContextWrapper
import android.content.res.Configuration
import android.content.res.Resources
import androidx.annotation.StringRes
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.langTagToLocale
import com.topjohnwu.superuser.internal.InternalUtils
import io.reactivex.Single
import java.util.*

var currentLocale = Locale.getDefault()!!
    private set

val defaultLocale = Locale.getDefault()!!

val availableLocales = Single.fromCallable {
    val compareId = R.string.app_changelog
    val res: Resources by inject()
    mutableListOf<Locale>().apply {
        // Add default locale
        add(Locale.ENGLISH)

        // Add some special locales
        add(Locale.TAIWAN)
        add(Locale("pt", "BR"))

        // Other locales
        val otherLocales = res.assets.locales
                .map { it.langTagToLocale() }
                .distinctBy { LocaleManager.getString(it, compareId) }

        listOf("", "").toTypedArray()

        addAll(otherLocales)
    }.sortedWith(Comparator { a, b ->
        a.getDisplayName(a).toLowerCase(a)
                .compareTo(b.getDisplayName(b).toLowerCase(b))
    })
}.cache()!!

object LocaleManager {

    fun setLocale(wrapper: ContextWrapper) {
        val localeConfig = Config.locale
        currentLocale = when {
            localeConfig.isEmpty() -> defaultLocale
            else -> localeConfig.langTagToLocale()
        }
        Locale.setDefault(currentLocale)
        InternalUtils.replaceBaseContext(wrapper, getLocaleContext(wrapper, currentLocale))
    }

    fun getLocaleContext(context: Context, locale: Locale = currentLocale): Context {
        val config = Configuration(context.resources.configuration)
        config.setLocale(locale)
        return context.createConfigurationContext(config)
    }

    fun getString(locale: Locale, @StringRes id: Int): String {
        return getLocaleContext(get(), locale).getString(id)
    }
}
