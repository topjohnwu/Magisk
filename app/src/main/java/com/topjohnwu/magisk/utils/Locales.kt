package com.topjohnwu.magisk.utils

import android.annotation.SuppressLint
import android.content.res.Configuration
import android.content.res.Resources
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ResourceMgr
import com.topjohnwu.magisk.extensions.langTagToLocale
import io.reactivex.Single
import java.util.*
import kotlin.Comparator

var currentLocale: Locale = Locale.getDefault()

@SuppressLint("ConstantLocale")
val defaultLocale: Locale = Locale.getDefault()

@Suppress("DEPRECATION")
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
