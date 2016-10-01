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
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplication());
        if (prefs.getString("theme","").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }

        Logger.devLog = prefs.getBoolean("developer_logging", false);
        Logger.logShell = prefs.getBoolean("shell_logging", false);

        // Initialize
        prefs.edit()
                .putBoolean("module_done", false)
                .putBoolean("repo_done", false)
                .putBoolean("update_check_done", false)
                .putBoolean("root", Utils.rootEnabled())
                .apply();

        new Async.CheckUpdates(prefs).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        new Async.constructEnv(getApplicationInfo()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

        new Async.LoadModules(prefs) {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(getApplicationContext()).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                // Start main activity
                Intent intent = new Intent(getApplicationContext(), MainActivity.class);
                startActivity(intent);
                finish();
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

    }
}
