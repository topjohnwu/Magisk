package com.topjohnwu.magisk;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.preference.ListPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceFragmentCompat;
import android.support.v7.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.HideManager;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.IOException;
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SettingsActivity extends Activity implements Topic.Subscriber {

    @BindView(R.id.toolbar) Toolbar toolbar;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_StatusBar_Dark;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
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
            getSupportFragmentManager().beginTransaction().add(R.id.container, new SettingsFragment()).commit();
        }

    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.RELOAD_ACTIVITY};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        recreate();
    }

    public static class SettingsFragment extends PreferenceFragmentCompat
            implements SharedPreferences.OnSharedPreferenceChangeListener,
            Topic.Subscriber, Topic.AutoSubscriber {

        private SharedPreferences prefs;
        private PreferenceScreen prefScreen;

        private ListPreference updateChannel, suAccess, autoRes, suNotification,
                requestTimeout, multiuserMode, namespaceMode;
        private MagiskManager mm;
        private PreferenceCategory generalCatagory;

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.app_settings, rootKey);
            mm = Utils.getMagiskManager(getActivity());
            prefs = mm.prefs;
            prefScreen = getPreferenceScreen();

            generalCatagory = (PreferenceCategory) findPreference("general");
            PreferenceCategory magiskCategory = (PreferenceCategory) findPreference("magisk");
            PreferenceCategory suCategory = (PreferenceCategory) findPreference("superuser");
            Preference hideManager = findPreference("hide");
            Preference restoreManager = findPreference("restore");
            findPreference("clear").setOnPreferenceClickListener((pref) -> {
                prefs.edit().remove(Const.Key.ETAG_KEY).apply();
                mm.repoDB.clearRepo();
                Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
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
            SwitchPreference fingerprint = (SwitchPreference) findPreference(Const.Key.SU_FINGERPRINT);

            updateChannel.setOnPreferenceChangeListener((pref, o) -> {
                Data.updateChannel = Integer.parseInt((String) o);
                if (Data.updateChannel == Const.Value.CUSTOM_CHANNEL) {
                    View v = LayoutInflater.from(getActivity()).inflate(R.layout.custom_channel_dialog, null);
                    EditText url = v.findViewById(R.id.custom_url);
                    url.setText(mm.prefs.getString(Const.Key.CUSTOM_CHANNEL, ""));
                    new AlertDialog.Builder(getActivity())
                            .setTitle(R.string.settings_update_custom)
                            .setView(v)
                            .setPositiveButton(R.string.ok, (d, i) ->
                                    prefs.edit().putString(Const.Key.CUSTOM_CHANNEL,
                                            url.getText().toString()).apply())
                            .setNegativeButton(R.string.close, null)
                            .show();
                }
                return true;
            });

            setSummary();

            // Disable dangerous settings in secondary user
            if (Const.USER_ID > 0) {
                suCategory.removePreference(multiuserMode);
            }

            // Disable re-authentication option on Android O, it will not work
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                reauth.setEnabled(false);
                reauth.setSummary(R.string.android_o_not_support);
            }

            // Disable fingerprint option if not possible
            if (!FingerprintHelper.canUseFingerprint()) {
                fingerprint.setEnabled(false);
                fingerprint.setSummary(R.string.disable_fingerprint);
            }

            if (Data.magiskVersionCode >= Const.MAGISK_VER.MANAGER_HIDE) {
                if (mm.getPackageName().equals(Const.ORIG_PKG_NAME)) {
                    hideManager.setOnPreferenceClickListener((pref) -> {
                        new HideManager(getActivity()).exec();
                        return true;
                    });
                    generalCatagory.removePreference(restoreManager);
                } else {
                    if (Download.checkNetworkStatus(mm)) {
                        restoreManager.setOnPreferenceClickListener((pref) -> {
                            Download.receive(
                                getActivity(), new DownloadReceiver() {
                                    @Override
                                    public void onDownloadDone(Context context, Uri uri) {
                                        Data.exportPrefs();
                                        Shell.su("cp " + uri.getPath() + " /data/local/tmp/manager.apk").exec();
                                        if (ShellUtils.fastCmdResult("pm install /data/local/tmp/manager.apk")) {
                                            Shell.su("rm -f /data/local/tmp/manager.apk").exec();
                                            RootUtils.uninstallPkg(context.getPackageName());
                                            return;
                                        }
                                        Shell.su("rm -f /data/local/tmp/manager.apk").exec();
                                    }
                                },
                                Data.managerLink,
                                Utils.fmt("MagiskManager-v%s.apk", Data.remoteManagerVersionString)
                            );
                            return true;
                        });
                    } else {
                        generalCatagory.removePreference(restoreManager);
                    }
                    generalCatagory.removePreference(hideManager);
                }
            } else {
                generalCatagory.removePreference(restoreManager);
                generalCatagory.removePreference(hideManager);
            }

            if (!Shell.rootAccess() || (Const.USER_ID > 0 &&
                    Data.multiuserMode == Const.Value.MULTIUSER_MODE_OWNER_MANAGED)) {
                prefScreen.removePreference(suCategory);
            }

            if (!Shell.rootAccess()) {
                prefScreen.removePreference(magiskCategory);
                generalCatagory.removePreference(hideManager);
            } else if (Data.magiskVersionCode < Const.MAGISK_VER.UNIFIED) {
                prefScreen.removePreference(magiskCategory);
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
                entryValues[i++] = locale.toLanguageTag();
            }
            lp.setEntries(entries);
            lp.setEntryValues(entryValues);
            lp.setSummary(LocaleManager.locale.getDisplayName(LocaleManager.locale));
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            prefs.registerOnSharedPreferenceChangeListener(this);
            Topic.subscribe(this);
            return super.onCreateView(inflater, container, savedInstanceState);
        }

        @Override
        public void onDestroyView() {
            prefs.unregisterOnSharedPreferenceChangeListener(this);
            Topic.unsubscribe(this);
            super.onDestroyView();
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {

            switch (key) {
                case Const.Key.DARK_THEME:
                    Data.isDarkTheme = prefs.getBoolean(key, false);
                    Topic.publish(false, Topic.RELOAD_ACTIVITY);
                    return;
                case Const.Key.COREONLY:
                    if (prefs.getBoolean(key, false)) {
                        try {
                            Const.MAGISK_DISABLE_FILE.createNewFile();
                        } catch (IOException ignored) {}
                    } else {
                        Const.MAGISK_DISABLE_FILE.delete();
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case Const.Key.MAGISKHIDE:
                    if (prefs.getBoolean(key, false)) {
                        Shell.su("magiskhide --enable").submit();
                    } else {
                        Shell.su("magiskhide --disable").submit();
                    }
                    break;
                case Const.Key.HOSTS:
                    if (prefs.getBoolean(key, false)) {
                        Shell.su("cp -af /system/etc/hosts " + Const.MAGISK_HOST_FILE,
                                "mount -o bind " + Const.MAGISK_HOST_FILE + " /system/etc/hosts")
                                .submit();
                    } else {
                        Shell.su("umount -l /system/etc/hosts",
                                "rm -f " + Const.MAGISK_HOST_FILE)
                                .submit();
                    }
                    break;
                case Const.Key.ROOT_ACCESS:
                case Const.Key.SU_MULTIUSER_MODE:
                case Const.Key.SU_MNT_NS:
                    mm.mDB.setSettings(key, Utils.getPrefsInt(prefs, key));
                    break;
                case Const.Key.LOCALE:
                    LocaleManager.setLocale(mm);
                    Topic.publish(false, Topic.RELOAD_ACTIVITY);
                    break;
                case Const.Key.UPDATE_CHANNEL:
                    new CheckUpdates().exec();
                    break;
                case Const.Key.CHECK_UPDATES:
                    Utils.setupUpdateCheck();
                    break;
            }
            Data.loadConfig();
            setSummary();
        }

        private void setSummary() {
            updateChannel.setSummary(getResources()
                    .getStringArray(R.array.update_channel)[Data.updateChannel]);
            suAccess.setSummary(getResources()
                    .getStringArray(R.array.su_access)[Data.suAccessState]);
            autoRes.setSummary(getResources()
                    .getStringArray(R.array.auto_response)[Data.suResponseType]);
            suNotification.setSummary(getResources()
                    .getStringArray(R.array.su_notification)[Data.suNotificationType]);
            requestTimeout.setSummary(
                    getString(R.string.request_timeout_summary, prefs.getString(Const.Key.SU_REQUEST_TIMEOUT, "10")));
            multiuserMode.setSummary(getResources()
                    .getStringArray(R.array.multiuser_summary)[Data.multiuserMode]);
            namespaceMode.setSummary(getResources()
                    .getStringArray(R.array.namespace_summary)[Data.suNamespaceMode]);
        }

        @Override
        public void onPublish(int topic, Object[] result) {
            setLocalePreference((ListPreference) findPreference(Const.Key.LOCALE));
        }

        @Override
        public int[] getSubscribedTopics() {
            return new int[] {Topic.LOCAL_FETCH_DONE};
        }
    }

}
