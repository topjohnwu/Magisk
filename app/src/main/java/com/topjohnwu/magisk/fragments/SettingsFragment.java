package com.topjohnwu.magisk.fragments;

import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreferenceCompat;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BasePreferenceFragment;
import com.topjohnwu.magisk.dialogs.FingerprintAuthDialog;
import com.topjohnwu.magisk.tasks.CheckUpdates;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.PatchAPK;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.io.IOException;
import java.util.Arrays;
import java.util.Locale;

public class SettingsFragment extends BasePreferenceFragment {

    private ListPreference updateChannel, autoRes, suNotification,
            requestTimeout, rootConfig, multiuserConfig, nsConfig;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.app_settings, rootKey);
        requireActivity().setTitle(R.string.settings);

        boolean showSuperuser = Utils.showSuperUser();
        app.prefs.edit()
                .putBoolean(Config.Key.SU_FINGERPRINT, FingerprintHelper.useFingerprint())
                .apply();

        PreferenceScreen prefScreen = getPreferenceScreen();

        PreferenceCategory generalCatagory = (PreferenceCategory) findPreference("general");
        PreferenceCategory magiskCategory = (PreferenceCategory) findPreference("magisk");
        PreferenceCategory suCategory = (PreferenceCategory) findPreference("superuser");
        Preference hideManager = findPreference("hide");
        hideManager.setOnPreferenceClickListener(pref -> {
            PatchAPK.hideManager();
            return true;
        });
        Preference restoreManager = findPreference("restore");
        restoreManager.setOnPreferenceClickListener(pref -> {
            DownloadApp.restore();
            return true;
        });
        findPreference("clear").setOnPreferenceClickListener(pref -> {
            app.prefs.edit().remove(Config.Key.ETAG_KEY).apply();
            app.repoDB.clearRepo();
            Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
            return true;
        });
        findPreference("hosts").setOnPreferenceClickListener(pref -> {
            Shell.su("add_hosts_module").exec();
            Utils.loadModules();
            Utils.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT);
            return true;
        });

        updateChannel = (ListPreference) findPreference(Config.Key.UPDATE_CHANNEL);
        rootConfig = (ListPreference) findPreference(Config.Key.ROOT_ACCESS);
        autoRes = (ListPreference) findPreference(Config.Key.SU_AUTO_RESPONSE);
        requestTimeout = (ListPreference) findPreference(Config.Key.SU_REQUEST_TIMEOUT);
        suNotification = (ListPreference) findPreference(Config.Key.SU_NOTIFICATION);
        multiuserConfig = (ListPreference) findPreference(Config.Key.SU_MULTIUSER_MODE);
        nsConfig = (ListPreference) findPreference(Config.Key.SU_MNT_NS);
        SwitchPreferenceCompat reauth = (SwitchPreferenceCompat) findPreference(Config.Key.SU_REAUTH);
        SwitchPreferenceCompat fingerprint = (SwitchPreferenceCompat) findPreference(Config.Key.SU_FINGERPRINT);

        updateChannel.setOnPreferenceChangeListener((p, o) -> {
            int prev = Config.get(Config.Key.UPDATE_CHANNEL);
            int channel = Integer.parseInt((String) o);
            if (channel == Config.Value.CUSTOM_CHANNEL) {
                View v = LayoutInflater.from(requireActivity()).inflate(R.layout.custom_channel_dialog, null);
                EditText url = v.findViewById(R.id.custom_url);
                url.setText(app.prefs.getString(Config.Key.CUSTOM_CHANNEL, ""));
                new AlertDialog.Builder(requireActivity())
                        .setTitle(R.string.settings_update_custom)
                        .setView(v)
                        .setPositiveButton(R.string.ok, (d, i) ->
                               Config.set(Config.Key.CUSTOM_CHANNEL, url.getText().toString()))
                        .setNegativeButton(R.string.close, (d, i) ->
                                Config.set(Config.Key.UPDATE_CHANNEL, prev))
                        .setOnCancelListener(d ->
                                Config.set(Config.Key.UPDATE_CHANNEL, prev))
                        .show();
            }
            return true;
        });

        /* We only show canary channels if user is already on canary channel
         * or the user have already chosen canary channel */
        if (!Utils.isCanary() &&
                (int) Config.get(Config.Key.UPDATE_CHANNEL) < Config.Value.CANARY_CHANNEL) {
            // Remove the last 2 entries
            CharSequence[] entries = updateChannel.getEntries();
            updateChannel.setEntries(Arrays.copyOf(entries, entries.length - 2));
        }

        setSummary();

        // Disable dangerous settings in secondary user
        if (Const.USER_ID > 0) {
            suCategory.removePreference(multiuserConfig);
        }

        // Disable re-authentication option on Android O, it will not work
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            reauth.setEnabled(false);
            reauth.setChecked(false);
            reauth.setSummary(R.string.android_o_not_support);
        }

        // Disable fingerprint option if not possible
        if (!FingerprintHelper.canUseFingerprint()) {
            fingerprint.setEnabled(false);
            fingerprint.setChecked(false);
            fingerprint.setSummary(R.string.disable_fingerprint);
        }

        if (Shell.rootAccess() && Const.USER_ID == 0) {
            if (app.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
                generalCatagory.removePreference(restoreManager);
            } else {
                if (!Networking.checkNetworkStatus(app))
                    generalCatagory.removePreference(restoreManager);
                generalCatagory.removePreference(hideManager);
            }
        } else {
            generalCatagory.removePreference(restoreManager);
            generalCatagory.removePreference(hideManager);
        }

        if (!showSuperuser) {
            prefScreen.removePreference(suCategory);
        }

        if (!Shell.rootAccess()) {
            prefScreen.removePreference(magiskCategory);
            generalCatagory.removePreference(hideManager);
        }
    }

    private void setLocalePreference(ListPreference lp) {
        CharSequence[] entries = new CharSequence[LocaleManager.locales.size() + 1];
        CharSequence[] entryValues = new CharSequence[LocaleManager.locales.size() + 1];
        entries[0] = LocaleManager.getString(LocaleManager.defaultLocale, R.string.system_default);
        entryValues[0] = "";
        int i = 1;
        for (Locale locale : LocaleManager.locales) {
            entries[i] = locale.getDisplayName(locale);
            entryValues[i++] = LocaleManager.toLanguageTag(locale);
        }
        lp.setEntries(entries);
        lp.setEntryValues(entryValues);
        lp.setSummary(LocaleManager.locale.getDisplayName(LocaleManager.locale));
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
        switch (key) {
            case Config.Key.ROOT_ACCESS:
            case Config.Key.SU_MULTIUSER_MODE:
            case Config.Key.SU_MNT_NS:
                app.mDB.setSettings(key, Utils.getPrefsInt(prefs, key));
                break;
            case Config.Key.DARK_THEME:
                requireActivity().recreate();
                break;
            case Config.Key.COREONLY:
                if (prefs.getBoolean(key, false)) {
                    try {
                        Const.MAGISK_DISABLE_FILE.createNewFile();
                    } catch (IOException ignored) {}
                } else {
                    Const.MAGISK_DISABLE_FILE.delete();
                }
                Utils.toast(R.string.settings_reboot_toast, Toast.LENGTH_LONG);
                break;
            case Config.Key.MAGISKHIDE:
                if (prefs.getBoolean(key, false)) {
                    Shell.su("magiskhide --enable").submit();
                } else {
                    Shell.su("magiskhide --disable").submit();
                }
                break;
            case Config.Key.LOCALE:
                LocaleManager.setLocale(app);
                requireActivity().recreate();
                break;
            case Config.Key.UPDATE_CHANNEL:
            case Config.Key.CUSTOM_CHANNEL:
                CheckUpdates.check();
                break;
            case Config.Key.CHECK_UPDATES:
                Utils.scheduleUpdateCheck();
                break;
        }
        setSummary(key);
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        String key = preference.getKey();
        switch (key) {
            case Config.Key.SU_FINGERPRINT:
                boolean checked = ((SwitchPreferenceCompat) preference).isChecked();
                ((SwitchPreferenceCompat) preference).setChecked(!checked);
                new FingerprintAuthDialog(requireActivity(), () -> {
                    ((SwitchPreferenceCompat) preference).setChecked(checked);
                    Config.set(key, checked);
                }).show();
                break;
        }
        return true;
    }

    private void setSummary(String key) {
        switch (key) {
            case Config.Key.UPDATE_CHANNEL:
                updateChannel.setSummary(getResources()
                        .getStringArray(R.array.update_channel)
                        [(int) Config.get(Config.Key.UPDATE_CHANNEL)]);
                break;
            case Config.Key.ROOT_ACCESS:
                rootConfig.setSummary(getResources()
                        .getStringArray(R.array.su_access)
                        [(int) Config.get(Config.Key.ROOT_ACCESS)]);
                break;
            case Config.Key.SU_AUTO_RESPONSE:
                autoRes.setSummary(getResources()
                        .getStringArray(R.array.auto_response)
                        [(int) Config.get(Config.Key.SU_AUTO_RESPONSE)]);
                break;
            case Config.Key.SU_NOTIFICATION:
                suNotification.setSummary(getResources()
                        .getStringArray(R.array.su_notification)
                        [(int) Config.get(Config.Key.SU_NOTIFICATION)]);
                break;
            case Config.Key.SU_REQUEST_TIMEOUT:
                requestTimeout.setSummary(
                        getString(R.string.request_timeout_summary,
                                (int) Config.get(Config.Key.SU_REQUEST_TIMEOUT)));
                break;
            case Config.Key.SU_MULTIUSER_MODE:
                multiuserConfig.setSummary(getResources()
                        .getStringArray(R.array.multiuser_summary)
                        [(int) Config.get(Config.Key.SU_MULTIUSER_MODE)]);
                break;
            case Config.Key.SU_MNT_NS:
                nsConfig.setSummary(getResources()
                        .getStringArray(R.array.namespace_summary)
                        [(int) Config.get(Config.Key.SU_MNT_NS)]);
                break;
        }
    }

    private void setSummary() {
        setSummary(Config.Key.UPDATE_CHANNEL);
        setSummary(Config.Key.ROOT_ACCESS);
        setSummary(Config.Key.SU_AUTO_RESPONSE);
        setSummary(Config.Key.SU_NOTIFICATION);
        setSummary(Config.Key.SU_REQUEST_TIMEOUT);
        setSummary(Config.Key.SU_MULTIUSER_MODE);
        setSummary(Config.Key.SU_MNT_NS);
    }

    @Override
    public void onEvent(int event) {
        setLocalePreference((ListPreference) findPreference(Config.Key.LOCALE));
    }

    @Override
    public int[] getListeningEvents() {
        return new int[] {Event.LOCALE_FETCH_DONE};
    }
}
