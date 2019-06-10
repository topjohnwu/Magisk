package com.topjohnwu.magisk.ui.settings

import android.content.SharedPreferences
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.edit
import androidx.preference.ListPreference
import androidx.preference.Preference
import androidx.preference.PreferenceCategory
import androidx.preference.SwitchPreferenceCompat
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoDatabaseHelper
import com.topjohnwu.magisk.ui.base.BasePreferenceFragment
import com.topjohnwu.magisk.utils.*
import com.topjohnwu.magisk.view.dialogs.FingerprintAuthDialog
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import org.koin.android.ext.android.inject

class SettingsFragment : BasePreferenceFragment() {

    private val repoDatabase: RepoDatabaseHelper by inject()

    private lateinit var updateChannel: ListPreference
    private lateinit var autoRes: ListPreference
    private lateinit var suNotification: ListPreference
    private lateinit var requestTimeout: ListPreference
    private lateinit var rootConfig: ListPreference
    private lateinit var multiuserConfig: ListPreference
    private lateinit var nsConfig: ListPreference

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.settings)
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        preferenceManager.setStorageDeviceProtected()
        setPreferencesFromResource(R.xml.app_settings, rootKey)

        updateChannel = findPref(Config.Key.UPDATE_CHANNEL)
        rootConfig = findPref(Config.Key.ROOT_ACCESS)
        autoRes = findPref(Config.Key.SU_AUTO_RESPONSE)
        requestTimeout = findPref(Config.Key.SU_REQUEST_TIMEOUT)
        suNotification = findPref(Config.Key.SU_NOTIFICATION)
        multiuserConfig = findPref(Config.Key.SU_MULTIUSER_MODE)
        nsConfig = findPref(Config.Key.SU_MNT_NS)
        val reauth = findPreference(Config.Key.SU_REAUTH) as SwitchPreferenceCompat
        val fingerprint = findPreference(Config.Key.SU_FINGERPRINT) as SwitchPreferenceCompat
        val generalCatagory = findPreference("general") as PreferenceCategory
        val magiskCategory = findPreference("magisk") as PreferenceCategory
        val suCategory = findPreference("superuser") as PreferenceCategory
        val hideManager = findPreference("hide")
        hideManager.setOnPreferenceClickListener {
            PatchAPK.hideManager()
            true
        }
        val restoreManager = findPreference("restore")
        restoreManager.setOnPreferenceClickListener {
            DownloadApp.restore()
            true
        }
        findPreference("clear").setOnPreferenceClickListener {
            prefs.edit {
                remove(Config.Key.ETAG_KEY)
            }
            repoDatabase.clearRepo()
            Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT)
            true
        }
        findPreference("hosts").setOnPreferenceClickListener {
            Shell.su("add_hosts_module").exec()
            Utils.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
            true
        }

        updateChannel.setOnPreferenceChangeListener { _, value ->
            val channel = Integer.parseInt(value as String)
            val previous = Config.updateChannel

            if (channel == Config.Value.CUSTOM_CHANNEL) {
                val v = LayoutInflater.from(requireActivity())
                        .inflate(R.layout.custom_channel_dialog, null)
                val url = v.findViewById<EditText>(R.id.custom_url)
                url.setText(Config.customChannelUrl)
                AlertDialog.Builder(requireActivity())
                        .setTitle(R.string.settings_update_custom)
                        .setView(v)
                        .setPositiveButton(R.string.ok) { _, _ ->
                            Config.customChannelUrl = url.text.toString() }
                        .setNegativeButton(R.string.close) { _, _ ->
                            Config.updateChannel = previous }
                        .setOnCancelListener { Config.updateChannel = previous }
                        .show()
            }
            true
        }

        setLocalePreference(findPreference(Config.Key.LOCALE) as ListPreference)

        /* We only show canary channels if user is already on canary channel
         * or the user have already chosen canary channel */
        if (!Utils.isCanary && Config.updateChannel < Config.Value.CANARY_CHANNEL) {
            // Remove the last 2 entries
            val entries = updateChannel.entries
            updateChannel.entries = entries.copyOf(entries.size - 2)

        }

        setSummary()

        // Disable dangerous settings in secondary user
        if (Const.USER_ID > 0) {
            suCategory.removePreference(multiuserConfig)
        }

        // Disable re-authentication option on Android O, it will not work
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            reauth.isEnabled = false
            reauth.isChecked = false
            reauth.setSummary(R.string.android_o_not_support)
        }

        // Disable fingerprint option if not possible
        if (!FingerprintHelper.canUseFingerprint()) {
            fingerprint.isEnabled = false
            fingerprint.isChecked = false
            fingerprint.setSummary(R.string.disable_fingerprint)
        }

        if (Shell.rootAccess() && Const.USER_ID == 0) {
            if (app.packageName == BuildConfig.APPLICATION_ID) {
                generalCatagory.removePreference(restoreManager)
            } else {
                if (!Networking.checkNetworkStatus(requireContext())) {
                    generalCatagory.removePreference(restoreManager)
                }
                generalCatagory.removePreference(hideManager)
            }
        } else {
            generalCatagory.removePreference(restoreManager)
            generalCatagory.removePreference(hideManager)
        }

        if (!Utils.showSuperUser()) {
            preferenceScreen.removePreference(suCategory)
        }

        if (!Shell.rootAccess()) {
            preferenceScreen.removePreference(magiskCategory)
            generalCatagory.removePreference(hideManager)
        }
    }

    override fun onSharedPreferenceChanged(prefs: SharedPreferences, key: String) {
        when (key) {
            Config.Key.ROOT_ACCESS -> Config.rootMode = Utils.getPrefsInt(prefs, key)
            Config.Key.SU_MULTIUSER_MODE -> Config.suMultiuserMode = Utils.getPrefsInt(prefs, key)
            Config.Key.SU_MNT_NS -> Config.suMntNamespaceMode = Utils.getPrefsInt(prefs, key)
            Config.Key.DARK_THEME -> requireActivity().recreate()
            Config.Key.COREONLY -> {
                if (prefs.getBoolean(key, false)) {
                    runCatching {
                        Const.MAGISK_DISABLE_FILE.createNewFile()
                    }
                } else {
                    Const.MAGISK_DISABLE_FILE.delete()
                }
                Utils.toast(R.string.settings_reboot_toast, Toast.LENGTH_LONG)
            }
            Config.Key.MAGISKHIDE -> if (prefs.getBoolean(key, false)) {
                Shell.su("magiskhide --enable").submit()
            } else {
                Shell.su("magiskhide --disable").submit()
            }
            Config.Key.LOCALE -> {
                LocaleManager.setLocale(app)
                requireActivity().recreate()
            }
            Config.Key.CHECK_UPDATES -> Utils.scheduleUpdateCheck()
        }
        setSummary(key)
    }

    override fun onPreferenceTreeClick(preference: Preference): Boolean {
        when (preference.key) {
            Config.Key.SU_FINGERPRINT -> {
                val checked = (preference as SwitchPreferenceCompat).isChecked
                preference.isChecked = !checked
                FingerprintAuthDialog(requireActivity()) {
                    preference.isChecked = checked
                    Config.suFingerprint = checked
                }.show()
            }
        }
        return true
    }

    private fun setLocalePreference(lp: ListPreference) {
        lp.isEnabled = false
        LocaleManager.availableLocales
                .map {
                    val names = mutableListOf<String>()
                    val values = mutableListOf<String>()

                    names.add(LocaleManager.getString(
                            LocaleManager.defaultLocale, R.string.system_default))
                    values.add("")

                    it.forEach { locale ->
                        names.add(locale.getDisplayName(locale))
                        values.add(LocaleManager.toLanguageTag(locale))
                    }

                    Pair(names.toTypedArray(), values.toTypedArray())
                }.subscribeK { (names, values) ->
                    lp.isEnabled = true
                    lp.entries = names
                    lp.entryValues = values
                    lp.summary = LocaleManager.locale.getDisplayName(LocaleManager.locale)
                }
    }

    private fun setSummary(key: String) {
        when (key) {
            Config.Key.ROOT_ACCESS -> rootConfig.summary = resources
                    .getStringArray(R.array.su_access)[Config.rootMode]
            Config.Key.SU_MULTIUSER_MODE -> multiuserConfig.summary = resources
                    .getStringArray(R.array.multiuser_summary)[Config.suMultiuserMode]
            Config.Key.SU_MNT_NS -> nsConfig.summary = resources
                    .getStringArray(R.array.namespace_summary)[Config.suMntNamespaceMode]
            Config.Key.UPDATE_CHANNEL -> {
                var ch = Config.updateChannel
                ch = if (ch < 0) Config.Value.STABLE_CHANNEL else ch
                updateChannel.summary = resources
                        .getStringArray(R.array.update_channel)[ch]
            }
            Config.Key.SU_AUTO_RESPONSE -> autoRes.summary = resources
                    .getStringArray(R.array.auto_response)[Config.suAutoReponse]
            Config.Key.SU_NOTIFICATION -> suNotification.summary = resources
                    .getStringArray(R.array.su_notification)[Config.suNotification]
            Config.Key.SU_REQUEST_TIMEOUT -> requestTimeout.summary =
                    getString(R.string.request_timeout_summary, Config.suDefaultTimeout)
        }
    }

    private fun setSummary() {
        setSummary(Config.Key.ROOT_ACCESS)
        setSummary(Config.Key.SU_MULTIUSER_MODE)
        setSummary(Config.Key.SU_MNT_NS)
        setSummary(Config.Key.UPDATE_CHANNEL)
        setSummary(Config.Key.SU_AUTO_RESPONSE)
        setSummary(Config.Key.SU_NOTIFICATION)
        setSummary(Config.Key.SU_REQUEST_TIMEOUT)
    }
}
