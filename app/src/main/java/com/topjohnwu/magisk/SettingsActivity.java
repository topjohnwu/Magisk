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
import com.topjohnwu.magisk.utils.Const;
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
    public void onTopicPublished(Topic topic, Object result) {
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
            Preference hideManager = findPreference("hide");
            findPreference("clear").setOnPreferenceClickListener((pref) -> {
                mm.prefs.edit().remove(Const.Key.ETAG_KEY).apply();
                mm.repoDB.clearRepo();
                MagiskManager.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
                return true;
            });

            updateChannel = (ListPreference) findPreference(Const.Key.UPDATE_CHANNEL);
            suAccess = (ListPreference) findPreference(Const.Key.ROOT_ACCESS);
            autoRes = (ListPreference) findPreference(Const.Key.SU_AUTO_RESPONSE);
            requestTimeout = (ListPreference) findPreference(Const.Key.SU_REQUEST_TIMEOUT);
            suNotification = (ListPreference) findPreference(Const.Key.SU_NOTIFICATION);
            multiuserMode = (ListPreference) findPreference(Const.Key.SU_MULTIUSER_MODE);
            namespaceMode = (ListPreference) findPreference(Const.Key.SU_MNT_NS);
            SwitchPreference reauth = (SwitchPreference) findPreference(Const.Key.SU_REAUTH);


            setSummary();

            // Disable dangerous settings in user mode if selected owner manage
            if (mm.userId > 0) {
                suCategory.removePreference(multiuserMode);
            }

            // Remove re-authentication option on Android O, it will not work
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                suCategory.removePreference(reauth);
            }

            if (mm.getPackageName().equals(Const.ORIG_PKG_NAME)) {
                hideManager.setOnPreferenceClickListener((pref) -> {
                    Utils.runWithPermission(getActivity(),
                            Manifest.permission.WRITE_EXTERNAL_STORAGE,
                            () -> new HideManager().exec());
                    return true;
                });
            } else {
                generalCatagory.removePreference(hideManager);
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
            lp.setKey(Const.Key.LOCALE);
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
            boolean enabled;

            switch (key) {
                case Const.Key.DARK_THEME:
                    enabled = prefs.getBoolean(Const.Key.DARK_THEME, false);
                    if (mm.isDarkTheme != enabled) {
                        mm.reloadActivity.publish(false);
                    }
                    break;
                case Const.Key.DISABLE:
                    enabled = prefs.getBoolean(Const.Key.DISABLE, false);
                    if (enabled) {
                        Utils.createFile(Const.MAGISK_DISABLE_FILE);
                    } else {
                        Utils.removeItem(Const.MAGISK_DISABLE_FILE);
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case Const.Key.MAGISKHIDE:
                    enabled = prefs.getBoolean(Const.Key.MAGISKHIDE, false);
                    if (enabled) {
                        Shell.su_raw("magiskhide --enable");
                    } else {
                        Shell.su_raw("magiskhide --disable");
                    }
                    break;
                case Const.Key.HOSTS:
                    enabled = prefs.getBoolean(Const.Key.HOSTS, false);
                    if (enabled) {
                        Shell.su_raw(
                                "cp -af /system/etc/hosts " + Const.MAGISK_HOST_FILE(),
                                "mount -o bind " + Const.MAGISK_HOST_FILE() + " /system/etc/hosts");
                    } else {
                        Shell.su_raw(
                                "umount -l /system/etc/hosts",
                                "rm -f " + Const.MAGISK_HOST_FILE());
                    }
                    break;
                case Const.Key.ROOT_ACCESS:
                    mm.suDB.setSettings(Const.Key.ROOT_ACCESS, Utils.getPrefsInt(prefs, Const.Key.ROOT_ACCESS));
                    break;
                case Const.Key.SU_MULTIUSER_MODE:
                    mm.suDB.setSettings(Const.Key.SU_MULTIUSER_MODE, Utils.getPrefsInt(prefs, Const.Key.SU_MULTIUSER_MODE));
                    break;
                case Const.Key.SU_MNT_NS:
                    mm.suDB.setSettings(Const.Key.SU_MNT_NS, Utils.getPrefsInt(prefs, Const.Key.SU_MNT_NS));
                    break;
                case Const.Key.LOCALE:
                    mm.setLocale();
                    mm.reloadActivity.publish(false);
                    break;
                case Const.Key.UPDATE_CHANNEL:
                    mm.updateChannel = Utils.getPrefsInt(prefs, Const.Key.UPDATE_CHANNEL);
                    new CheckUpdates(true).exec();
                    break;
            }
            mm.loadConfig();
            setSummary();
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
                    getString(R.string.request_timeout_summary, prefs.getString(Const.Key.SU_REQUEST_TIMEOUT, "10")));
            multiuserMode.setSummary(getResources()
                    .getStringArray(R.array.multiuser_summary)[mm.multiuserMode]);
            namespaceMode.setSummary(getResources()
                    .getStringArray(R.array.namespace_summary)[mm.suNamespaceMode]);
        }

        @Override
        public void onTopicPublished(Topic topic, Object result) {
            setLocalePreference((ListPreference) findPreference(Const.Key.LOCALE));
        }

        @Override
        public Topic[] getSubscription() {
            return new Topic[] { mm.localeDone };
        }
    }

}
