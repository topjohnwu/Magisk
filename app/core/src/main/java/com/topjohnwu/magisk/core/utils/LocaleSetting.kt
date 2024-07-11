package com.topjohnwu.magisk.core.utils

import android.annotation.SuppressLint
import android.app.LocaleConfig
import android.app.LocaleManager
import android.content.ContextWrapper
import android.content.res.Resources
import android.os.Build
import android.os.LocaleList
import androidx.annotation.RequiresApi
import com.topjohnwu.magisk.core.AppApkPath
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.base.relaunch
import com.topjohnwu.magisk.core.isRunningAsStub
import org.xmlpull.v1.XmlPullParser
import java.util.Locale

interface LocaleSetting {
    // The locale that is manually overridden, null if system default
    val appLocale: Locale?
    // The current active locale used in the application
    val currentLocale: Locale

    fun setLocale(tag: String)
    fun updateResource(res: Resources)

    private class Api23Impl : LocaleSetting {

        private val systemLocale: Locale = Locale.getDefault()

        override var currentLocale: Locale = systemLocale
        override var appLocale: Locale? = null

        init {
            setLocale(Config.locale)
        }

        override fun setLocale(tag: String) {
            val locale = when {
                tag.isEmpty() -> null
                else -> Locale.forLanguageTag(tag)
            }
            currentLocale = locale ?: systemLocale
            appLocale = locale
            Locale.setDefault(currentLocale)
            updateResource(AppContext.resources)
            AppContext.foregroundActivity?.relaunch()
        }

        @Suppress("DEPRECATION")
        override fun updateResource(res: Resources) {
            val config = res.configuration
            config.setLocale(currentLocale)
            res.updateConfiguration(config, null)
        }
    }

    @RequiresApi(24)
    private class Api24Impl : LocaleSetting {

        private val systemLocaleList = LocaleList.getDefault()
        private var currentLocaleList: LocaleList = systemLocaleList

        override var appLocale: Locale? = null
        override val currentLocale: Locale get() = currentLocaleList[0]

        init {
            setLocale(Config.locale)
        }

        override fun setLocale(tag: String) {
            val localeList = when {
                tag.isEmpty() -> null
                else -> LocaleList.forLanguageTags(tag)
            }
            currentLocaleList = localeList ?: systemLocaleList
            appLocale = localeList?.get(0)
            LocaleList.setDefault(currentLocaleList)
            updateResource(AppContext.resources)
            AppContext.foregroundActivity?.relaunch()
        }

        @Suppress("DEPRECATION")
        override fun updateResource(res: Resources) {
            val config = res.configuration
            config.setLocales(currentLocaleList)
            res.updateConfiguration(config, null)
        }
    }

    @RequiresApi(33)
    private class Api33Impl : LocaleSetting {

        private val lm: LocaleManager = AppContext.getSystemService(LocaleManager::class.java)

        override val appLocale: Locale?
            get() = lm.applicationLocales.let { if (it.isEmpty) null else it[0] }

        override val currentLocale: Locale
            get() = appLocale ?: lm.systemLocales[0]

        // These following methods should not be used
        override fun setLocale(tag: String) {}
        override fun updateResource(res: Resources) {}
    }

    class AppLocaleList(
        val names: Array<String>,
        val tags: Array<String>
    )

    @SuppressLint("NewApi")
    companion object {
        val available: AppLocaleList by lazy {
            val names = ArrayList<String>()
            val tags = ArrayList<String>()

            names.add(AppContext.getString(R.string.system_default))
            tags.add("")

            if (Build.VERSION.SDK_INT >= 34) {
                // Use platform LocaleConfig parser
                val config = localeConfig
                val list = config.supportedLocales ?: LocaleList.getEmptyLocaleList()
                names.ensureCapacity(list.size() + 1)
                tags.ensureCapacity(list.size() + 1)
                for (i in 0 until list.size()) {
                    val locale = list[i]
                    names.add(locale.getDisplayName(locale))
                    tags.add(locale.toLanguageTag())
                }
            } else {
                // Manually parse locale_config.xml
                val parser = AppContext.resources.getXml(R.xml.locale_config)
                while (true) {
                    when (parser.next()) {
                        XmlPullParser.START_TAG -> {
                            if (parser.name == "locale") {
                                val tag = parser.getAttributeValue(0)
                                val locale = Locale.forLanguageTag(tag)
                                names.add(locale.getDisplayName(locale))
                                tags.add(tag)
                            }
                        }
                        XmlPullParser.END_DOCUMENT -> break
                    }
                }
            }
            AppLocaleList(names.toTypedArray(), tags.toTypedArray())
        }

        @get:RequiresApi(34)
        val localeConfig: LocaleConfig by lazy {
            val context = if (isRunningAsStub) {
                val pkgInfo = AppContext.packageManager.getPackageArchiveInfo(AppApkPath, 0)!!
                object : ContextWrapper(AppContext) {
                    override fun getApplicationInfo() = pkgInfo.applicationInfo
                }
            } else {
                AppContext
            }
            LocaleConfig.fromContextIgnoringOverride(context)
        }

        val useLocaleManager get() =
            if (isRunningAsStub) Build.VERSION.SDK_INT >= 34
            else Build.VERSION.SDK_INT >= 33

        val instance: LocaleSetting by lazy {
            // Initialize available locale list
            available
            if (useLocaleManager) {
                Api33Impl()
            } else if (Build.VERSION.SDK_INT <= 23) {
                Api23Impl()
            } else {
                Api24Impl()
            }
        }
    }
}
