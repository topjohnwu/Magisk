package com.topjohnwu.magisk.core

import android.os.Bundle
import androidx.core.content.edit
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.repository.DBConfig
import com.topjohnwu.magisk.core.repository.PreferenceConfig
import com.topjohnwu.magisk.core.utils.LocaleSetting
import kotlinx.coroutines.GlobalScope

object Config : PreferenceConfig, DBConfig {

    override val stringDB get() = ServiceLocator.stringDB
    override val settingsDB get() = ServiceLocator.settingsDB
    override val context get() = ServiceLocator.deContext
    override val coroutineScope get() = GlobalScope

    object Key {
        // db configs
        const val ROOT_ACCESS = "root_access"
        const val SU_MULTIUSER_MODE = "multiuser_mode"
        const val SU_MNT_NS = "mnt_ns"
        const val SU_BIOMETRIC = "su_biometric"
        const val ZYGISK = "zygisk"
        const val BOOTLOOP = "bootloop"
        const val SU_MANAGER = "requester"
        const val KEYSTORE = "keystore"

        // prefs
        const val SU_REQUEST_TIMEOUT = "su_request_timeout"
        const val SU_AUTO_RESPONSE = "su_auto_response"
        const val SU_NOTIFICATION = "su_notification"
        const val SU_REAUTH = "su_reauth"
        const val SU_TAPJACK = "su_tapjack"
        const val SU_RESTRICT = "su_restrict"
        const val CHECK_UPDATES = "check_update"
        const val RELEASE_CHANNEL = "release_channel"
        const val CUSTOM_CHANNEL = "custom_channel"
        const val LOCALE = "locale"
        const val DARK_THEME = "dark_theme_extended"
        const val DOWNLOAD_DIR = "download_dir"
        const val SAFETY = "safety_notice"
        const val THEME_ORDINAL = "theme_ordinal"
        const val ASKED_HOME = "asked_home"
        const val DOH = "doh"
        const val RAND_NAME = "rand_name"

        val NO_MIGRATION = setOf(ASKED_HOME, SU_REQUEST_TIMEOUT,
            SU_AUTO_RESPONSE, SU_REAUTH, SU_TAPJACK)
    }

    object OldValue {
        // Update channels
        const val DEFAULT_CHANNEL = -1
        const val STABLE_CHANNEL = 0
        const val BETA_CHANNEL = 1
        const val CUSTOM_CHANNEL = 2
        const val CANARY_CHANNEL = 3
        const val DEBUG_CHANNEL = 4
    }

    object Value {
        // Update channels
        const val DEFAULT_CHANNEL = -1
        const val STABLE_CHANNEL = 0
        const val BETA_CHANNEL = 1
        const val DEBUG_CHANNEL = 2
        const val CUSTOM_CHANNEL = 3

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
        val TIMEOUT_LIST = longArrayOf(0, -1, 10, 20, 30, 60)
    }

    @JvmField var keepVerity = false
    @JvmField var keepEnc = false
    @JvmField var recovery = false
    var denyList = false

    var askedHome by preference(Key.ASKED_HOME, false)
    var bootloop by dbSettings(Key.BOOTLOOP, 0)

    var safetyNotice by preference(Key.SAFETY, true)
    var darkTheme by preference(Key.DARK_THEME, -1)
    var themeOrdinal by preference(Key.THEME_ORDINAL, 0)

    private var checkUpdatePrefs by preference(Key.CHECK_UPDATES, true)
    private var localePrefs by preference(Key.LOCALE, "")
    var doh by preference(Key.DOH, false)
    var updateChannel by preference(Key.RELEASE_CHANNEL, Value.DEFAULT_CHANNEL)
    var customChannelUrl by preference(Key.CUSTOM_CHANNEL, "")
    var downloadDir by preference(Key.DOWNLOAD_DIR, "")
    var randName by preference(Key.RAND_NAME, true)
    var checkUpdate
        get() = checkUpdatePrefs
        set(value) {
            if (checkUpdatePrefs != value) {
                checkUpdatePrefs = value
                JobService.schedule(AppContext)
            }
        }
    var locale
        get() = localePrefs
        set(value) {
            localePrefs = value
            LocaleSetting.instance.setLocale(value)
        }

    var zygisk by dbSettings(Key.ZYGISK, Info.isEmulator)
    var suManager by dbStrings(Key.SU_MANAGER, "", true)
    var keyStoreRaw by dbStrings(Key.KEYSTORE, "", true)

    var suDefaultTimeout by preferenceStrInt(Key.SU_REQUEST_TIMEOUT, 10)
    var suAutoResponse by preferenceStrInt(Key.SU_AUTO_RESPONSE, Value.SU_PROMPT)
    var suNotification by preferenceStrInt(Key.SU_NOTIFICATION, Value.NOTIFICATION_TOAST)
    var rootMode by dbSettings(Key.ROOT_ACCESS, Value.ROOT_ACCESS_APPS_AND_ADB)
    var suMntNamespaceMode by dbSettings(Key.SU_MNT_NS, Value.NAMESPACE_MODE_REQUESTER)
    var suMultiuserMode by dbSettings(Key.SU_MULTIUSER_MODE, Value.MULTIUSER_MODE_OWNER_ONLY)
    private var suBiometric by dbSettings(Key.SU_BIOMETRIC, false)
    var suAuth
        get() = Info.isDeviceSecure && suBiometric
        set(value) {
            suBiometric = value
        }
    var suReAuth by preference(Key.SU_REAUTH, false)
    var suTapjack by preference(Key.SU_TAPJACK, true)
    var suRestrict by preference(Key.SU_RESTRICT, false)

    private const val SU_FINGERPRINT = "su_fingerprint"
    private const val UPDATE_CHANNEL = "update_channel"

    fun toBundle(): Bundle {
        val map = prefs.all - Key.NO_MIGRATION
        return Bundle().apply {
            for ((key, value) in map) {
                when (value) {
                    is String -> putString(key, value)
                    is Int -> putInt(key, value)
                    is Boolean -> putBoolean(key, value)
                }
            }
        }
    }

    @Suppress("DEPRECATION")
    private fun fromBundle(bundle: Bundle) {
        val keys = bundle.keySet().apply { removeAll(Key.NO_MIGRATION) }
        prefs.edit {
            for (key in keys) {
                when (val value = bundle.get(key)) {
                    is String -> putString(key, value)
                    is Int -> putInt(key, value)
                    is Boolean -> putBoolean(key, value)
                }
            }
        }
    }

    fun init(bundle: Bundle?) {
        // Only try to load prefs when fresh install
        if (bundle != null && prefs.all.isEmpty()) {
            fromBundle(bundle)
        }

        prefs.edit {
            // Migrate su_fingerprint
            if (prefs.getBoolean(SU_FINGERPRINT, false))
                suBiometric = true
            remove(SU_FINGERPRINT)

            // Migrate update_channel
            prefs.getString(UPDATE_CHANNEL, null)?.let {
                val channel = when (it.toInt()) {
                    OldValue.STABLE_CHANNEL -> Value.STABLE_CHANNEL
                    OldValue.CANARY_CHANNEL, OldValue.BETA_CHANNEL -> Value.BETA_CHANNEL
                    OldValue.DEBUG_CHANNEL -> Value.DEBUG_CHANNEL
                    OldValue.CUSTOM_CHANNEL -> Value.CUSTOM_CHANNEL
                    else -> Value.DEFAULT_CHANNEL
                }
                putInt(Key.RELEASE_CHANNEL, channel)
            }
            remove(UPDATE_CHANNEL)
        }
    }
}
