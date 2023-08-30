package com.topjohnwu.magisk.core

import android.annotation.SuppressLint
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.edit
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.di.AppContext
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.repository.BoolDBPropertyNoWrite
import com.topjohnwu.magisk.core.repository.DBConfig
import com.topjohnwu.magisk.core.repository.PreferenceConfig
import com.topjohnwu.magisk.core.utils.refreshLocale
import com.topjohnwu.magisk.ui.theme.Theme
import kotlinx.coroutines.GlobalScope
import java.io.File

object Config : PreferenceConfig, DBConfig {

    override val stringDB get() = ServiceLocator.stringDB
    override val settingsDB get() = ServiceLocator.settingsDB
    override val context get() = ServiceLocator.deContext
    override val coroutineScope get() = GlobalScope

    private val prefsFile = File("${context.filesDir.parent}/shared_prefs", "${fileName}.xml")

    @SuppressLint("ApplySharedPref")
    fun getPrefsFile(): File {
        prefs.edit().remove(Key.ASKED_HOME).commit()
        return prefsFile
    }

    object Key {
        // db configs
        const val ROOT_ACCESS = "root_access"
        const val SU_MULTIUSER_MODE = "multiuser_mode"
        const val SU_MNT_NS = "mnt_ns"
        const val SU_BIOMETRIC = "su_biometric"
        const val ZYGISK = "zygisk"
        const val DENYLIST = "denylist"
        const val SU_MANAGER = "requester"
        const val KEYSTORE = "keystore"

        // prefs
        const val SU_REQUEST_TIMEOUT = "su_request_timeout"
        const val SU_AUTO_RESPONSE = "su_auto_response"
        const val SU_NOTIFICATION = "su_notification"
        const val SU_REAUTH = "su_reauth"
        const val SU_TAPJACK = "su_tapjack"
        const val CHECK_UPDATES = "check_update"
        const val UPDATE_CHANNEL = "update_channel"
        const val CUSTOM_CHANNEL = "custom_channel"
        const val LOCALE = "locale"
        const val DARK_THEME = "dark_theme_extended"
        const val REPO_ORDER = "repo_order"
        const val SHOW_SYSTEM_APP = "show_system"
        const val DOWNLOAD_DIR = "download_dir"
        const val SAFETY = "safety_notice"
        const val THEME_ORDINAL = "theme_ordinal"
        const val BOOT_ID = "boot_id"
        const val ASKED_HOME = "asked_home"
        const val DOH = "doh"
    }

    object Value {
        // Update channels
        const val DEFAULT_CHANNEL = -1
        const val STABLE_CHANNEL = 0
        const val BETA_CHANNEL = 1
        const val CUSTOM_CHANNEL = 2
        const val CANARY_CHANNEL = 3
        const val DEBUG_CHANNEL = 4

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
        if (BuildConfig.DEBUG)
            Value.DEBUG_CHANNEL
        else if (Const.APP_IS_CANARY)
            Value.CANARY_CHANNEL
        else
            Value.DEFAULT_CHANNEL

    @JvmField var keepVerity = false
    @JvmField var keepEnc = false
    @JvmField var recovery = false

    var bootId by preference(Key.BOOT_ID, "")
    var askedHome by preference(Key.ASKED_HOME, false)

    var downloadDir by preference(Key.DOWNLOAD_DIR, "")
    var repoOrder by preference(Key.REPO_ORDER, Value.ORDER_DATE)

    var suDefaultTimeout by preferenceStrInt(Key.SU_REQUEST_TIMEOUT, 10)
    var suAutoResponse by preferenceStrInt(Key.SU_AUTO_RESPONSE, Value.SU_PROMPT)
    var suNotification by preferenceStrInt(Key.SU_NOTIFICATION, Value.NOTIFICATION_TOAST)
    var updateChannel by preferenceStrInt(Key.UPDATE_CHANNEL, defaultChannel)

    var safetyNotice by preference(Key.SAFETY, true)
    var darkTheme by preference(Key.DARK_THEME, AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM)
    var themeOrdinal by preference(Key.THEME_ORDINAL, Theme.Piplup.ordinal)
    var suReAuth by preference(Key.SU_REAUTH, false)
    var suTapjack by preference(Key.SU_TAPJACK, true)
    private var checkUpdatePrefs by preference(Key.CHECK_UPDATES, true)
    var checkUpdate
        get() = checkUpdatePrefs
        set(value) {
            if (checkUpdatePrefs != value) {
                checkUpdatePrefs = value
                JobService.schedule(AppContext)
            }
        }
    var doh by preference(Key.DOH, false)
    var showSystemApp by preference(Key.SHOW_SYSTEM_APP, false)

    var customChannelUrl by preference(Key.CUSTOM_CHANNEL, "")
    private var localePrefs by preference(Key.LOCALE, "")
    var locale
        get() = localePrefs
        set(value) {
            localePrefs = value
            refreshLocale()
        }

    var rootMode by dbSettings(Key.ROOT_ACCESS, Value.ROOT_ACCESS_APPS_AND_ADB)
    var suMntNamespaceMode by dbSettings(Key.SU_MNT_NS, Value.NAMESPACE_MODE_REQUESTER)
    var suMultiuserMode by dbSettings(Key.SU_MULTIUSER_MODE, Value.MULTIUSER_MODE_OWNER_ONLY)
    var suBiometric by dbSettings(Key.SU_BIOMETRIC, false)
    var zygisk by dbSettings(Key.ZYGISK, false)
    var denyList by BoolDBPropertyNoWrite(Key.DENYLIST, false)
    var suManager by dbStrings(Key.SU_MANAGER, "", true)
    var keyStoreRaw by dbStrings(Key.KEYSTORE, "", true)

    private const val SU_FINGERPRINT = "su_fingerprint"

    fun load(pkg: String?) {
        // Only try to load prefs when fresh install and a previous package name is set
        if (pkg != null && prefs.all.isEmpty()) runCatching {
            context.contentResolver.openInputStream(Provider.preferencesUri(pkg))?.writeTo(prefsFile)
            return
        }

        prefs.edit {
            // Settings migration
            if (prefs.getBoolean(SU_FINGERPRINT, false))
                suBiometric = true
            remove(SU_FINGERPRINT)
            prefs.getString(Key.UPDATE_CHANNEL, null).also {
                if (it == null ||
                    it.toInt() > Value.DEBUG_CHANNEL ||
                    it.toInt() < Value.DEFAULT_CHANNEL) {
                    putString(Key.UPDATE_CHANNEL, defaultChannel.toString())
                }
            }
        }
    }
}
