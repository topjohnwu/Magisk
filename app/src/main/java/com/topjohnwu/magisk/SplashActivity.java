package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.services.MonitorService;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import java.util.HashSet;
import java.util.Set;

public class SplashActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences(getApplication());
        if (defaultPrefs.getString("theme","").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }

        Logger.devLog = defaultPrefs.getBoolean("developer_logging", false);
        Logger.logShell = defaultPrefs.getBoolean("shell_logging", false);

        // Set up default preferences,make sure we add "extra" blacklist entries.
        if (!defaultPrefs.contains("auto_blacklist")) {
            Logger.dev("SplashActivity: Setting default preferences for application");
            SharedPreferences.Editor editor = defaultPrefs.edit();
            Set<String> set = new HashSet<>();
            set.add("com.google.android.apps.walletnfcrel");
            set.add("com.google.android.gms");
            set.add("com.google.commerce.tapandpay");
            editor.putStringSet("auto_blacklist", set);
            editor.putBoolean("autoRootEnable", false);
            editor.putBoolean("root", Utils.rootEnabled());
            editor.apply();
        }

        // Set up toggle states based on preferences, start services, disable root if set
        if (Utils.autoToggleEnabled(getApplicationContext())) {
            if (!Utils.hasServicePermission(getApplicationContext())) {
                Utils.toggleAutoRoot(false, getApplicationContext());
            }
        }
        if (Utils.autoToggleEnabled(getApplicationContext())) {
            if (!Utils.isMyServiceRunning(MonitorService.class, getApplicationContext())) {
                Intent myIntent = new Intent(getApplication(), MonitorService.class);
                getApplication().startService(myIntent);
            }
        } else if (defaultPrefs.getBoolean("keep_root_off", false)) {
            Utils.toggleRoot(false, getApplication());
        }

        // Set up quick settings tile
        Utils.setupQuickSettingsTile(getApplicationContext());

        // Initialize
        Utils.init(this);

        defaultPrefs.edit()
                .putBoolean("module_done", false)
                .putBoolean("repo_done", false)
                .putBoolean("update_check_done", false)
                .apply();

        new Async.CheckUpdates(this).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        new Async.constructEnv(this).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);

        new Async.LoadModules(this).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        new Async.LoadRepos(this).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

        // Start main activity
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);

        finish();
    }
}
