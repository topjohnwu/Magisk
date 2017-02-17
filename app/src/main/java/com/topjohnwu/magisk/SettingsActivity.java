package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.view.WindowManager;
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.MagiskHide;
import com.topjohnwu.magisk.asyncs.SerialTask;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SettingsActivity extends Activity {

    @BindView(R.id.toolbar) Toolbar toolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getApplicationContext().isDarkTheme) {
            setTheme(R.style.AppTheme_Dark);
        }

        setContentView(R.layout.activity_container);
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

    public void setFloating() {
        boolean isTablet = getResources().getBoolean(R.bool.isTablet);
        if (isTablet) {
            WindowManager.LayoutParams params = getWindow().getAttributes();
            params.height = getResources().getDimensionPixelSize(R.dimen.floating_height);
            params.width = getResources().getDimensionPixelSize(R.dimen.floating_width);
            params.alpha = 1.0f;
            params.dimAmount = 0.6f;
            params.flags |= 2;
            getWindow().setAttributes(params);
            setFinishOnTouchOutside(true);
        }
    }

    public static class SettingsFragment extends PreferenceFragment
            implements SharedPreferences.OnSharedPreferenceChangeListener {

        private SharedPreferences prefs;
        private PreferenceScreen prefScreen;

        private ListPreference suAccess, autoRes, suNotification, requestTimeout;

        private MagiskManager getApplication() {
            return Utils.getMagiskManager(getActivity());
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.app_settings);
            prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
            prefScreen = getPreferenceScreen();

            SwitchPreference busybox = (SwitchPreference) findPreference("busybox");
            SwitchPreference magiskHide = (SwitchPreference) findPreference("magiskhide");
            SwitchPreference hosts = (SwitchPreference) findPreference("hosts");

            PreferenceCategory magiskCategory = (PreferenceCategory) findPreference("magisk");
            PreferenceCategory suCategory = (PreferenceCategory) findPreference("superuser");

            suAccess = (ListPreference) findPreference("su_access");
            autoRes = (ListPreference) findPreference("su_auto_response");
            requestTimeout = (ListPreference) findPreference("su_request_timeout");
            suNotification = (ListPreference) findPreference("su_notification");

            setSummary();

            findPreference("clear").setOnPreferenceClickListener((pref) -> {
                Utils.clearRepoCache(getActivity());
                return true;
            });

            if (!Shell.rootAccess()) {
                prefScreen.removePreference(magiskCategory);
                prefScreen.removePreference(suCategory);
            } else {
                if (!getApplication().isSuClient)
                    prefScreen.removePreference(suCategory);
                if (getApplication().magiskVersion < 11)
                    prefScreen.removePreference(magiskCategory);
                if (getApplication().disabled) {
                    busybox.setEnabled(false);
                    magiskHide.setEnabled(false);
                    hosts.setEnabled(false);
                }
            }
        }

        @Override
        public void onResume() {
            super.onResume();
            prefs.registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onPause() {
            super.onPause();
            prefs.unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            Logger.dev("Settings: Prefs change " + key);
            boolean enabled;

            switch (key) {
                case "dark_theme":
                    enabled = prefs.getBoolean("dark_theme", false);
                    if (getApplication().isDarkTheme != enabled) {
                        getApplication().isDarkTheme = enabled;
                        getApplication().reloadMainActivity.trigger();
                        getActivity().finish();
                        getActivity().recreate();
                    }
                    break;
                case "disable":
                    enabled = prefs.getBoolean("disable", false);
                    new SerialTask<Void, Void, Void>() {
                        private boolean enable = enabled;
                        @Override
                        protected Void doInBackground(Void... voids) {
                            if (enable) {
                                Utils.createFile(MagiskManager.MAGISK_DISABLE_FILE);
                            } else {
                                Utils.removeItem(MagiskManager.MAGISK_DISABLE_FILE);
                            }
                            return null;
                        }
                    }.exec();
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case "busybox":
                    enabled = prefs.getBoolean("busybox", false);
                    new SerialTask<Void, Void, Void>() {
                        private boolean enable = enabled;
                        @Override
                        protected Void doInBackground(Void... voids) {
                            if (enable) {
                                Shell.su(
                                        "setprop persist.magisk.busybox 1",
                                        "sh /sbin/magic_mask.sh mount_busybox");
                            } else {
                                Shell.su(
                                        "setprop persist.magisk.busybox 0",
                                        "umount /system/xbin");
                            }
                            return null;
                        }
                    }.exec();
                    break;
                case "magiskhide":
                    enabled = prefs.getBoolean("magiskhide", false);
                    if (enabled) {
                        if (!getApplication().isSuClient) {
                            new AlertDialogBuilder(getActivity())
                                    .setTitle(R.string.no_magisksu_title)
                                    .setMessage(R.string.no_magisksu_msg)
                                    .setPositiveButton(R.string.understand, (dialog, which) -> new MagiskHide().enable())
                                    .setCancelable(false)
                                    .show();
                        } else new MagiskHide().enable();
                    } else
                        new MagiskHide().disable();
                    break;
                case "hosts":
                    enabled = prefs.getBoolean("hosts", false);
                    new SerialTask<Void, Void, Void>() {
                        private boolean enable = enabled;
                        @Override
                        protected Void doInBackground(Void... voids) {
                            if (enable) {
                                Shell.su("cp -af /system/etc/hosts /magisk/.core/hosts",
                                        "mount -o bind /magisk/.core/hosts /system/etc/hosts");
                            } else {
                                Shell.su("umount -l /system/etc/hosts",
                                        "rm -f /magisk/.core/hosts");
                            }
                            return null;
                        }
                    }.exec();
                    break;
                case "su_access":
                    getApplication().suAccessState = Utils.getPrefsInt(prefs, "su_access", 0);
                    Shell.su("setprop persist.sys.root_access " + getApplication().suAccessState);
                    break;
                case "su_request_timeout":
                    getApplication().suRequestTimeout = Utils.getPrefsInt(prefs, "su_request_timeout", 10);
                    break;
                case "su_auto_response":
                    getApplication().suResponseType = Utils.getPrefsInt(prefs, "su_auto_response", 0);
                    break;
                case "su_notification":
                    getApplication().suNotificationType = Utils.getPrefsInt(prefs, "su_notification", 1);
                    break;
                case "developer_logging":
                    MagiskManager.devLogging = prefs.getBoolean("developer_logging", false);
                    break;
                case "shell_logging":
                    MagiskManager.shellLogging = prefs.getBoolean("shell_logging", false);
                    break;
            }
            setSummary();
        }

        private void setSummary() {
            suAccess.setSummary(getResources()
                    .getStringArray(R.array.su_access)[getApplication().suAccessState]);
            autoRes.setSummary(getResources()
                    .getStringArray(R.array.auto_response)[getApplication().suResponseType]);
            suNotification.setSummary(getResources()
                    .getStringArray(R.array.su_notification)[getApplication().suNotificationType]);
            requestTimeout.setSummary(
                    getString(R.string.request_timeout_summary, prefs.getString("su_request_timeout", "10")));
        }
    }

}
