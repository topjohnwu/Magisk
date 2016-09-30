package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

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

        // Initialize
        Utils.init(this);

        defaultPrefs.edit()
                .putBoolean("module_done", false)
                .putBoolean("repo_done", false)
                .putBoolean("update_check_done", false)
                .putBoolean("root", Utils.rootEnabled())
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
