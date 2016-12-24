package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.WindowManager;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SettingsActivity extends AppCompatActivity {

    @BindView(R.id.toolbar) Toolbar toolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String theme = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString("theme", "");
        Logger.dev("AboutActivity: Theme is " + theme);
        if (theme.equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
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

        private ListPreference themePreference;
        private SharedPreferences prefs;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.app_settings);
            PreferenceManager.setDefaultValues(getActivity(), R.xml.app_settings, false);
            prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

            themePreference = (ListPreference) findPreference("theme");
            CheckBoxPreference busyboxPreference = (CheckBoxPreference) findPreference("busybox");
            CheckBoxPreference magiskhidePreference = (CheckBoxPreference) findPreference("magiskhide");
            CheckBoxPreference hostsPreference = (CheckBoxPreference) findPreference("hosts");

            themePreference.setSummary(themePreference.getValue());

            if (StatusFragment.magiskVersion < 9) {
                hostsPreference.setEnabled(false);
                busyboxPreference.setEnabled(false);
            } else if (StatusFragment.magiskVersion < 8) {
                magiskhidePreference.setEnabled(false);
            } else {
                busyboxPreference.setEnabled(true);
                magiskhidePreference.setEnabled(true);
                hostsPreference.setEnabled(true);
            }
        }

        @Override
        public void onResume() {
            super.onResume();
            prefs.registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            prefs.unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            Logger.dev("Settings: Prefs change " + key);
            boolean checked;

            switch (key) {
                case "theme":
                    String theme = prefs.getString(key, "");

                    themePreference.setSummary(theme);
                    if (theme.equals("Dark")) {
                        getActivity().getApplication().setTheme(R.style.AppTheme_dh);
                    } else {
                        getActivity().getApplication().setTheme(R.style.AppTheme);
                    }
                    Intent intent = new Intent(getActivity(), MainActivity.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                    break;
                case "magiskhide":
                     checked = prefs.getBoolean("magiskhide", false);
                    if (checked) {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... params) {
                                Utils.createFile("/magisk/.core/magiskhide/enable");
                                return null;
                            }
                        }.exec();
                    } else {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... params) {
                                Utils.removeItem("/magisk/.core/magiskhide/enable");
                                return null;
                            }
                        }.exec();
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case "busybox":
                    checked = prefs.getBoolean("busybox", false);
                    if (checked) {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... params) {
                                Utils.createFile("/magisk/.core/busybox/enable");
                                return null;
                            }
                        }.exec();
                    } else {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... params) {
                                Utils.removeItem("/magisk/.core/busybox/enable");
                                return null;
                            }
                        }.exec();
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case "hosts":
                    checked = prefs.getBoolean("hosts", false);
                    if (checked) {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... voids) {
                                Shell.su("cp -af /system/etc/hosts /magisk/.core/hosts");
                                return null;
                            }
                        }.exec();
                    } else {
                        new Async.RootTask<Void, Void, Void>() {
                            @Override
                            protected Void doInBackground(Void... voids) {
                                Shell.su("umount -l /system/etc/hosts", "rm -f /magisk/.core/hosts");
                                return null;
                            }
                        }.exec();
                    }
                    Toast.makeText(getActivity(), R.string.settings_reboot_toast, Toast.LENGTH_LONG).show();
                    break;
                case "developer_logging":
                    Logger.devLog = prefs.getBoolean("developer_logging", false);
                    break;
                case "shell_logging":
                    Logger.logShell = prefs.getBoolean("shell_logging", false);
                    break;
            }

        }
    }

}
