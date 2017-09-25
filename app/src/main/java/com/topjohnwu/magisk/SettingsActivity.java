package com.topjohnwu.magisk;

import android.Manifest;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.HideManager;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SettingsActivity extends Activity implements Topic.Subscriber {

    @BindView(R.id.toolbar) Toolbar toolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getMagiskManager().isDarkTheme) {
            setTheme(R.style.AppTheme_Transparent_Dark);
        }

        setContentView(R.layout.activity_settings);
        ButterKnife.bind(this);

        setSupportActionBar(toolbar);

        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.settings);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        setFloating();

        if (savedInstanceState == null) {
            getFragmentManager().beginTransaction().add(R.id.container, new SettingsFragment()).commit();
        }

    }

    @Override
    public void onTopicPublished(Topic topic) {
        recreate();
    }

    @Override
    public Topic[] getSubscription() {
        return new Topic[] { getMagiskManager().reloadActivity };
    }

    public static class SettingsFragment extends PreferenceFragment
            implements SharedPreferences.OnSharedPreferenceChangeListener,
            Topic.Subscriber {

        private SharedPreferences prefs;
        private PreferenceScreen prefScreen;

        private ListPreference updateChannel, suAccess, autoRes, suNotification,
                requestTimeout, multiuserMode, namespaceMode;
        private MagiskManager mm;
        private PreferenceCategory generalCatagory;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.app_settings);
            prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
            prefScreen = getPreferenceScreen();
            mm = Utils.getMagiskManager(getActivity());

            generalCatagory = (PreferenceCategory) findPreference("general");
            PreferenceCategory magiskCategory = (PreferenceCategory) findPreference("magisk");
            PreferenceCategory suCategory = (PreferenceCategory) findPreference("superuser");
            PreferenceCategory developer = (PreferenceCategory) findPreference("developer");

            updateChannel = (ListPreference) findPreference("update_channel");
            suAccess = (ListPreference) findPreference("su_access");
            autoRes = (ListPreference) findPreference("su_auto_response");
            requestTimeout = (ListPreference) findPreference("su_request_timeout");
            suNotification = (ListPreference) findPreference("su_notification");
            multiuserMode = (ListPreference) findPreference("multiuser_mode");
            namespaceMode = (ListPreference) findPreference("mnt_ns");
            SwitchPreference reauth = (SwitchPreference) findPreference("su_reauth");
            Preference hideManager = findPreference("hide");

            setSummary();

            // Disable dangerous settings in user mode if selected owner manage
            if (getActivity().getApplicationInfo().uid > 99999) {
                prefScreen.removePreference(magiskCategory);
                prefScreen.removePreference(suCategory);
                generalCatagory.removePreference(hideManager);
            }

            // Remove re-authentication option on Android O, it will not work
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                suCategory.removePreference(reauth);
            }

            findPreference("clear").setOnPreferenceClickListener((pref) -> {
                Utils.clearRepoCache(getActivity());
                return true;
            });

            hideManager.setOnPreferenceClickListener((pref) -> {
                Utils.runWithPermission(getActivity(),
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        () -> new HideManager(getActivity()).exec());
                return true;
            });

            if (!BuildConfig.DEBUG) {
                prefScreen.removePreference(developer);
            }

            if (!Shell.rootAccess()) {
                prefScreen.removePreference(magiskCategory);
                prefScreen.removePreference(suCategory);
                generalCatagory.removePreference(hideManager);
            } else {
                if (!mm.isSuClient) {
                    prefScreen.removePreference(suCategory);
                }
                if (mm.magiskVersionCode < 1300) {
                    prefScreen.removePreference(magiskCategory);
                }
            }
        }

        private void setLocalePreference(ListPreference lp) {
            boolean isNew = lp == null;
            if (isNew) {
                lp = new ListPreference(getActivity());
            }
            CharSequence[] entries = new CharSequence[mm.locales.size() + 1];
            CharSequence[] entryValues = new CharSequence[mm.locales.size() + 1];
            entries[0] = getString(R.string.system_default);
            entryValues[0] = "";
            int i = 1;
            for (Locale locale : mm.locales) {
                entries[i] = locale.getDisplayName(locale);
                entryValues[i++] = locale.toLanguageTag();
            }
            lp.setEntries(entries);
            lp.setEntryValues(entryValues);
            lp.setTitle(R.string.language);
            lp.setKey("locale");
            lp.setSummary(MagiskManager.locale.getDisplayName(MagiskManager.locale));
            if (isNew) {
                generalCatagory.addPreference(lp);
            }
        }

        @Override
        public void onResume() {
            super.onResume();
            prefs.registerOnSharedPreferenceChangeListener(this);
            subscribeTopics();
        }

        @Override
        public void onPause() {
            prefs.unregisterOnSharedPreferenceChangeListener(this);
            unsubscribeTopics();
            super.onPause();
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            Logger.dev("Settings: Prefs change " + key);
            boolean enabled;

            switch (key) {
                case "dark_theme":
                    enabled = prefs.getBoolean("dark_theme", false);
                    if (mm.isDarkTheme != enabled) {
                        mm.reloadActivity.publish(false);
                    }
                    break;
                case "disable":
                    enabled = prefs.getBoolean("disable", false);
                    if (enabled) {
                        Utils.createFile(getShell(), MagiskManager.MAGISK_DISABLE_FILE);
                    } else {
                        Utils.removeItem(getShell(), MagiskManager.MAGISK_DISABLE_FILE);
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case "magiskhide":
                    enabled = prefs.getBoolean("magiskhide", false);
                    if (enabled) {
                        Utils.enableMagiskHide(getShell());
                    } else {
                        Utils.disableMagiskHide(getShell());
                    }
                    break;
                case "hosts":
                    enabled = prefs.getBoolean("hosts", false);
                    if (enabled) {
                        getShell().su_raw(
                                "cp -af /system/etc/hosts " + MagiskManager.MAGISK_HOST_FILE,
                                "mount -o bind " + MagiskManager.MAGISK_HOST_FILE + " /system/etc/hosts");
                    } else {
                        getShell().su_raw(
                                "umount -l /system/etc/hosts",
                                "rm -f " + MagiskManager.MAGISK_HOST_FILE);
                    }
                    break;
                case "su_access":
                    mm.suDB.setSettings(SuDatabaseHelper.ROOT_ACCESS, Utils.getPrefsInt(prefs, "su_access"));
                    break;
                case "multiuser_mode":
                    mm.suDB.setSettings(SuDatabaseHelper.MULTIUSER_MODE, Utils.getPrefsInt(prefs, "multiuser_mode"));
                    break;
                case "mnt_ns":
                    mm.suDB.setSettings(SuDatabaseHelper.MNT_NS, Utils.getPrefsInt(prefs, "mnt_ns"));
                    break;
                case "locale":
                    mm.setLocale();
                    mm.reloadActivity.publish(false);
                    break;
                case "update_channel":
                    mm.updateChannel = Utils.getPrefsInt(prefs, "update_channel");
                    new CheckUpdates(mm, true).exec();
                    break;
            }
            mm.loadConfig();
            setSummary();
        }

        private Shell getShell() {
            return Shell.getShell(getActivity());
        }

        private void setSummary() {
            updateChannel.setSummary(getResources()
                    .getStringArray(R.array.update_channel)[mm.updateChannel]);
            suAccess.setSummary(getResources()
                    .getStringArray(R.array.su_access)[mm.suAccessState]);
            autoRes.setSummary(getResources()
                    .getStringArray(R.array.auto_response)[mm.suResponseType]);
            suNotification.setSummary(getResources()
                    .getStringArray(R.array.su_notification)[mm.suNotificationType]);
            requestTimeout.setSummary(
                    getString(R.string.request_timeout_summary, prefs.getString("su_request_timeout", "10")));
            multiuserMode.setSummary(getResources()
                    .getStringArray(R.array.multiuser_summary)[mm.multiuserMode]);
            namespaceMode.setSummary(getResources()
                    .getStringArray(R.array.namespace_summary)[mm.suNamespaceMode]);
        }

        @Override
        public void onTopicPublished(Topic topic) {
            setLocalePreference((ListPreference) findPreference("locale"));
        }

        @Override
        public Topic[] getSubscription() {
            return new Topic[] { mm.localeDone };
        }
    }

}
