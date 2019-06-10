package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.ContextWrapper
import android.content.res.Configuration
import android.content.res.Resources
import android.os.Build
import androidx.annotation.StringRes
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.superuser.internal.InternalUtils
import io.reactivex.Single
import java.util.*

object LocaleManager {
    @JvmStatic
    var locale = Locale.getDefault()
    @JvmStatic
    val defaultLocale = Locale.getDefault()

    @JvmStatic
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
                .map { forLanguageTag(it) }
                .distinctBy { getString(it, compareId) }

            listOf("", "").toTypedArray()

            addAll(otherLocales)
        }.sortedWith(Comparator { a, b ->
            a.getDisplayName(a).toLowerCase(a)
                .compareTo(b.getDisplayName(b).toLowerCase(b))
        })
    }.cache()

    private fun forLanguageTag(tag: String): Locale {
        if (Build.VERSION.SDK_INT >= 21) {
            return Locale.forLanguageTag(tag)
        } else {
            val tok = tag.split("-".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
            if (tok.isEmpty()) {
                return Locale("")
            }
            val language = when (tok[0]) {
                "und" -> "" // Undefined
                "fil" -> "tl" // Filipino
                else -> tok[0]
            }
            if (language.length != 2 && language.length != 3)
                return Locale("")
            if (tok.size == 1)
                return Locale(language)
            val country = tok[1]

            return if (country.length != 2 && country.length != 3) Locale(language)
            else Locale(language, country)
        }
    }

    @JvmStatic
    fun toLanguageTag(loc: Locale): String {
        if (Build.VERSION.SDK_INT >= 21) {
            return loc.toLanguageTag()
        } else {
            var language = loc.language
            var country = loc.country
            var variant = loc.variant
            when {
                language.isEmpty() || !language.matches("\\p{Alpha}{2,8}".toRegex()) ->
                    language = "und"       // Follow the Locale#toLanguageTag() implementation
                language == "iw" -> language = "he"        // correct deprecated "Hebrew"
                language == "in" -> language = "id"        // correct deprecated "Indonesian"
                language == "ji" -> language = "yi"        // correct deprecated "Yiddish"
            }
            // ensure valid country code, if not well formed, it's omitted

            // variant subtags that begin with a letter must be at least 5 characters long
            // ensure valid country code, if not well formed, it's omitted
            if (!country.matches("\\p{Alpha}{2}|\\p{Digit}{3}".toRegex())) {
                country = ""
            }

            // variant subtags that begin with a letter must be at least 5 characters long
            if (!variant.matches("\\p{Alnum}{5,8}|\\p{Digit}\\p{Alnum}{3}".toRegex())) {
                variant = ""
            }
            val tag = StringBuilder(language)
            if (country.isNotEmpty())
                tag.append('-').append(country)
            if (variant.isNotEmpty())
                tag.append('-').append(variant)
            return tag.toString()
        }
    }

    @JvmStatic
    fun setLocale(wrapper: ContextWrapper) {
        val localeConfig = Config.locale
        locale = when {
            localeConfig.isEmpty() -> defaultLocale
            else -> forLanguageTag(localeConfig)
        }
        Locale.setDefault(locale)
        InternalUtils.replaceBaseContext(wrapper, getLocaleContext(locale))
    }

    @JvmStatic
    fun getLocaleContext(context: Context, locale: Locale): Context {
        val config = Configuration(context.resources.configuration)
        config.setLocale(locale)
        return context.createConfigurationContext(config)
    }

    @JvmStatic
    fun getLocaleContext(locale: Locale): Context {
        return getLocaleContext(App.self.baseContext, locale)
    }

    @JvmStatic
    fun getString(locale: Locale, @StringRes id: Int): String {
        return getLocaleContext(locale).getString(id)
    }
}
