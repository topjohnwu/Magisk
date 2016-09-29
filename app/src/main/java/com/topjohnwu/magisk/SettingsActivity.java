package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
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

    public static class SettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener {

        private ListPreference themePreference;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.uisettings);
            PreferenceManager.setDefaultValues(getActivity(), R.xml.uisettings, false);

            themePreference = (ListPreference) findPreference("theme");
            CheckBoxPreference busyboxPreference = (CheckBoxPreference) findPreference("busybox");
            busyboxPreference.setChecked(Utils.commandExists("unzip"));

            PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);

            themePreference.setSummary(themePreference.getValue());

            if (MagiskFragment.magiskVersion == -1) {
                busyboxPreference.setEnabled(false);
            } else {
                busyboxPreference.setEnabled(true);
            }
        }

        @Override
        public void onResume() {
            super.onResume();
            PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            PreferenceManager.getDefaultSharedPreferences(getActivity()).unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            Logger.dev("Settings: Prefs change " + key);

            switch (key) {
                case "theme":
                    String theme = sharedPreferences.getString(key, "");

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
                case "busybox":
                    boolean checked = sharedPreferences.getBoolean("busybox", false);
                    new Async.LinkBusyBox(checked).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                    break;
                case "developer_logging":
                    Logger.devLog = sharedPreferences.getBoolean("developer_logging", false);
                    break;
                case "shell_logging":
                    Logger.logShell = sharedPreferences.getBoolean("shell_logging", false);
                    break;
            }

        }
    }

}
