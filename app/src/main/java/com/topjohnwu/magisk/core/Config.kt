package com.topjohnwu.magisk.core

import android.content.Context
import android.content.SharedPreferences
import android.os.Environment
import android.util.Xml
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.edit
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.magiskdb.SettingsDao
import com.topjohnwu.magisk.core.magiskdb.StringDao
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.data.repository.DBConfig
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.model.preference.PreferenceModel
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFile
import com.topjohnwu.superuser.io.SuFileInputStream
import org.xmlpull.v1.XmlPullParser
import java.io.File

object Config : PreferenceModel, DBConfig {

    override val stringDao: StringDao by inject()
    override val settingsDao: SettingsDao by inject()
    override val context: Context by inject(Protected)

    object Key {
        // db configs
        const val ROOT_ACCESS = "root_access"
        const val SU_MULTIUSER_MODE = "multiuser_mode"
        const val SU_MNT_NS = "mnt_ns"
        const val SU_BIOMETRIC = "su_biometric"
        const val SU_MANAGER = "requester"
        const val KEYSTORE = "keystore"

        // prefs
        const val SU_REQUEST_TIMEOUT = "su_request_timeout"
        const val SU_AUTO_RESPONSE = "su_auto_response"
        const val SU_NOTIFICATION = "su_notification"
        const val SU_REAUTH = "su_reauth"
        const val CHECK_UPDATES = "check_update"
        const val UPDATE_CHANNEL = "update_channel"
        const val CUSTOM_CHANNEL = "custom_channel"
        const val LOCALE = "locale"
        const val DARK_THEME = "dark_theme"
        const val DARK_THEME_EXTENDED = "dark_theme_extended"
        const val REPO_ORDER = "repo_order"
        const val SHOW_SYSTEM_APP = "show_system"
        const val DOWNLOAD_PATH = "download_path"
        const val REDESIGN = "redesign"
        const val SAFETY = "safety_notice"
        const val THEME_ORDINAL = "theme_ordinal"
        const val BOOT_ID = "boot_id"
        const val LIST_SPAN_COUNT = "column_count"

        // system state
        const val MAGISKHIDE = "magiskhide"
        const val COREONLY = "disable"
    }

    object Value {
        // Update channels
        const val DEFAULT_CHANNEL = -1
        const val STABLE_CHANNEL = 0
        const val BETA_CHANNEL = 1
        const val CUSTOM_CHANNEL = 2
        const val CANARY_CHANNEL = 3
        const val CANARY_DEBUG_CHANNEL = 4

        // root access mode
        const val ROOT_ACCESS_DISABLED = 0
        const val ROOT_ACCESS_APPS_ONLY = 1
        const val ROOT_ACCESS_ADB_ONLY = 2
        const val ROOT_ACCESS_APPS_AND_ADB = 3

        // su multiuser
        const val MULTIUSER_MODE_OWNER_ONLY = 0
        const val MULTIUSER_MODE_OWNER_MANAGED = 1
        const val MULTIUSER_MODE_USER = 2

        // su mnt ns
        const val NAMESPACE_MODE_GLOBAL = 0
        const val NAMESPACE_MODE_REQUESTER = 1
        const val NAMESPACE_MODE_ISOLATE = 2

        // su notification
        const val NO_NOTIFICATION = 0
        const val NOTIFICATION_TOAST = 1

        // su auto response
        const val SU_PROMPT = 0
        const val SU_AUTO_DENY = 1
        const val SU_AUTO_ALLOW = 2

        // su timeout
        val TIMEOUT_LIST = intArrayOf(0, -1, 10, 20, 30, 60)

        // repo order
        const val ORDER_NAME = 0
        const val ORDER_DATE = 1
    }

    private val defaultChannel =
        if (isCanaryVersion) {
            if (BuildConfig.DEBUG)
                Value.CANARY_DEBUG_CHANNEL
            else
                Value.CANARY_CHANNEL
        } else Value.DEFAULT_CHANNEL

    var bootId by preference(Key.BOOT_ID, "")

    var downloadPath by preference(Key.DOWNLOAD_PATH, Environment.DIRECTORY_DOWNLOADS)
    var repoOrder by preference(Key.REPO_ORDER, Value.ORDER_DATE)

    var suDefaultTimeout by preferenceStrInt(Key.SU_REQUEST_TIMEOUT, 10)
    var suAutoReponse by preferenceStrInt(Key.SU_AUTO_RESPONSE, Value.SU_PROMPT)
    var suNotification by preferenceStrInt(Key.SU_NOTIFICATION, Value.NOTIFICATION_TOAST)
    var updateChannel by preferenceStrInt(Key.UPDATE_CHANNEL, defaultChannel)

    var safetyNotice by preference(Key.SAFETY, true)
    var darkThemeExtended by preference(
        Key.DARK_THEME_EXTENDED,
        AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM
    )
    var themeOrdinal by preference(Key.THEME_ORDINAL, Theme.Piplup.ordinal)
    var suReAuth by preference(Key.SU_REAUTH, false)
    var checkUpdate by preference(Key.CHECK_UPDATES, true)
    var magiskHide by preference(Key.MAGISKHIDE, true)
    @JvmStatic
    var coreOnly by preference(Key.COREONLY, false)
    var showSystemApp by preference(Key.SHOW_SYSTEM_APP, false)
    @JvmStatic
    var listSpanCount by preference(Key.LIST_SPAN_COUNT, 1)

    var customChannelUrl by preference(Key.CUSTOM_CHANNEL, "")
    var locale by preference(Key.LOCALE, "")

    var rootMode by dbSettings(Key.ROOT_ACCESS, Value.ROOT_ACCESS_APPS_AND_ADB)
    var suMntNamespaceMode by dbSettings(Key.SU_MNT_NS, Value.NAMESPACE_MODE_REQUESTER)
    var suMultiuserMode by dbSettings(Key.SU_MULTIUSER_MODE, Value.MULTIUSER_MODE_OWNER_ONLY)
    var suBiometric by dbSettings(Key.SU_BIOMETRIC, false)
    var suManager by dbStrings(Key.SU_MANAGER, "", true)
    var keyStoreRaw by dbStrings(Key.KEYSTORE, "", true)

    // Always return a path in external storage where we can write
    val downloadDirectory get() =
        Utils.ensureDownloadPath(downloadPath) ?: get<Context>().getExternalFilesDir(null)!!

    private const val SU_FINGERPRINT = "su_fingerprint"

    fun initialize() = prefs.also {
        if (it.getBoolean(SU_FINGERPRINT, false)) {
            suBiometric = true
        }
    }.edit {
        parsePrefs(this)

        // Legacy stuff
        remove(SU_FINGERPRINT)

        // Get actual state
        putBoolean(Key.COREONLY, Const.MAGISK_DISABLE_FILE.exists())

        // Write database configs
        putString(Key.ROOT_ACCESS, rootMode.toString())
        putString(Key.SU_MNT_NS, suMntNamespaceMode.toString())
        putString(Key.SU_MULTIUSER_MODE, suMultiuserMode.toString())
        putBoolean(Key.SU_BIOMETRIC, BiometricHelper.isEnabled)
    }.also {
        if (!prefs.contains(Key.UPDATE_CHANNEL))
            prefs.edit().putString(Key.UPDATE_CHANNEL, defaultChannel.toString()).apply()
    }

    private fun parsePrefs(editor: SharedPreferences.Editor) = editor.apply {
        val config = SuFile.open("/data/adb",
            Const.MANAGER_CONFIGS
        )
        if (config.exists()) runCatching {
            val input = SuFileInputStream(config)
            val parser = Xml.newPullParser()
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false)
            parser.setInput(input, "UTF-8")
            parser.nextTag()
            parser.require(XmlPullParser.START_TAG, null, "map")
            while (parser.next() != XmlPullParser.END_TAG) {
                if (parser.eventType != XmlPullParser.START_TAG)
                    continue
                val key: String = parser.getAttributeValue(null, "name")
                fun value() = parser.getAttributeValue(null, "value")!!
                when (parser.name) {
                    "string" -> {
                        parser.require(XmlPullParser.START_TAG, null, "string")
                        putString(key, parser.nextText())
                        parser.require(XmlPullParser.END_TAG, null, "string")
                    }
                    "boolean" -> {
                        parser.require(XmlPullParser.START_TAG, null, "boolean")
                        putBoolean(key, value().toBoolean())
                        parser.nextTag()
                        parser.require(XmlPullParser.END_TAG, null, "boolean")
                    }
                    "int" -> {
                        parser.require(XmlPullParser.START_TAG, null, "int")
                        putInt(key, value().toInt())
                        parser.nextTag()
                        parser.require(XmlPullParser.END_TAG, null, "int")
                    }
                    "long" -> {
                        parser.require(XmlPullParser.START_TAG, null, "long")
                        putLong(key, value().toLong())
                        parser.nextTag()
                        parser.require(XmlPullParser.END_TAG, null, "long")
                    }
                    "float" -> {
                        parser.require(XmlPullParser.START_TAG, null, "int")
                        putFloat(key, value().toFloat())
                        parser.nextTag()
                        parser.require(XmlPullParser.END_TAG, null, "int")
                    }
                    else -> parser.next()
                }
            }
            config.delete()
        }
    }

    fun export() {
        // Flush prefs to disk
        prefs.edit().commit()
        val context = get<Context>(Protected)
        val xml = File(
            "${context.filesDir.parent}/shared_prefs",
            "${context.packageName}_preferences.xml"
        )
        Shell.su("cat $xml > /data/adb/${Const.MANAGER_CONFIGS}").exec()
    }

}
