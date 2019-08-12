package com.topjohnwu.magisk.ui.settings

import android.content.SharedPreferences
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.view.LayoutInflater
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.databinding.DataBindingUtil
import androidx.preference.ListPreference
import androidx.preference.Preference
import androidx.preference.PreferenceCategory
import androidx.preference.SwitchPreferenceCompat
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.databinding.CustomDownloadDialogBinding
import com.topjohnwu.magisk.extensions.toLangTag
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.net.Networking
import com.topjohnwu.magisk.ui.base.BasePreferenceFragment
import com.topjohnwu.magisk.utils.*
import com.topjohnwu.magisk.view.dialogs.FingerprintAuthDialog
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import org.koin.android.ext.android.inject
import java.io.File

class SettingsFragment : BasePreferenceFragment() {

    private val repoDB: RepoDao by inject()

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
            PatchAPK.hideManager(requireContext())
            true
        }
        val restoreManager = findPreference("restore")
        restoreManager.setOnPreferenceClickListener {
            DownloadService(requireContext()) {
                subject = DownloadSubject.Manager(Configuration.APK.Restore)
            }
            true
        }
        findPreference("clear").setOnPreferenceClickListener {
            Completable.fromAction { repoDB.clear() }.subscribeK {
                Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT)
            }
            true
        }
        findPreference("hosts").setOnPreferenceClickListener {
            Shell.su("add_hosts_module").submit {
                Utils.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
            }
            true
        }

        findPreference(Config.Key.DOWNLOAD_PATH).apply {
            summary = Config.downloadPath
        }.setOnPreferenceClickListener { preference ->
            activity.withExternalRW {
                onSuccess {
                    showDownloadDialog {
                        Config.downloadPath = it
                        preference.summary = it
                    }
                }
            }
            true
        }

        updateChannel.setOnPreferenceChangeListener { _, value ->
            val channel = Integer.parseInt(value as String)
            val previous = Config.updateChannel

            if (channel == Config.Value.CUSTOM_CHANNEL) {
                showUrlDialog(Config.customChannelUrl, {
                    Config.updateChannel = previous
                }, {
                    Config.customChannelUrl = it
                })
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
            if (activity.packageName == BuildConfig.APPLICATION_ID) {
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
        fun getStrInt() = prefs.getString(key, null)?.toInt() ?: 0

        when (key) {
            Config.Key.ROOT_ACCESS -> Config.rootMode = getStrInt()
            Config.Key.SU_MULTIUSER_MODE -> Config.suMultiuserMode = getStrInt()
            Config.Key.SU_MNT_NS -> Config.suMntNamespaceMode = getStrInt()
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
                LocaleManager.setLocale(activity.application)
                activity.recreate()
            }
            Config.Key.CHECK_UPDATES -> Utils.scheduleUpdateCheck(activity)
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
        availableLocales.map {
                val names = mutableListOf<String>()
                val values = mutableListOf<String>()

                names.add(
                    LocaleManager.getString(defaultLocale, R.string.system_default)
                )
                values.add("")

                it.forEach { locale ->
                    names.add(locale.getDisplayName(locale))
                    values.add(locale.toLangTag())
                }

                Pair(names.toTypedArray(), values.toTypedArray())
            }.subscribeK { (names, values) ->
                lp.isEnabled = true
                lp.entries = names
                lp.entryValues = values
                lp.summary = currentLocale.getDisplayName(currentLocale)
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

    private inline fun showUrlDialog(
        initialValue: String,
        crossinline onCancel: () -> Unit = {},
        crossinline onSuccess: (String) -> Unit
    ) {
        val v = LayoutInflater
            .from(requireActivity())
            .inflate(R.layout.custom_channel_dialog, null)

        val url = v.findViewById<EditText>(R.id.custom_url).apply {
            setText(initialValue)
        }

        AlertDialog.Builder(requireActivity())
            .setTitle(R.string.settings_update_custom)
            .setView(v)
            .setPositiveButton(R.string.ok) { _, _ -> onSuccess(url.text.toString()) }
            .setNegativeButton(R.string.close) { _, _ -> onCancel() }
            .setOnCancelListener { onCancel() }
            .show()
    }

    inner class DownloadDialogData(initialValue: String) {
        val text = KObservableField(initialValue)
        val path = Observer(text) {
            File(Environment.getExternalStorageDirectory(), text.value).absolutePath
        }
    }

    private inline fun showDownloadDialog(
        initialValue: String = Config.downloadPath,
        crossinline onSuccess: (String) -> Unit
    ) {
        val data = DownloadDialogData(initialValue)
        val binding: CustomDownloadDialogBinding = DataBindingUtil
            .inflate(layoutInflater, R.layout.custom_download_dialog, null, false)
        binding.also { it.data = data }

        AlertDialog.Builder(requireActivity())
            .setTitle(R.string.settings_download_path_title)
            .setView(binding.root)
            .setPositiveButton(R.string.ok) { _, _ ->
                Utils.ensureDownloadPath(data.text.value)?.let { onSuccess(data.text.value) }
                    ?: Utils.toast(R.string.settings_download_path_error, Toast.LENGTH_SHORT)
            }
            .setNegativeButton(R.string.close, null)
            .show()
    }
}