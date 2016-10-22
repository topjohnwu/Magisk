package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class SplashActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplication());
        if (prefs.getString("theme", "").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }

        Logger.devLog = prefs.getBoolean("developer_logging", false);
        Logger.logShell = prefs.getBoolean("shell_logging", false);

        // Set up default preferences,make sure we add "extra" blacklist entries.
        int hideVersion = Utils.WhichHide(getApplication());
        List<String> hideList;
        Set<String> set = new HashSet<>();
        switch (hideVersion) {
            case 1:
                hideList = Shell.su("/magisk/.core/magiskhide/list");
                set.addAll(hideList);
                break;
            case 2:
                hideList = Shell.su("/su/suhide/list");
                break;
            case 3:
                hideList = Shell.su("/magisk/.core/magiskhide/list");
                hideList.addAll(Shell.su("/su/suhide/list"));
                set.addAll(hideList);
        }
        if (!prefs.contains("auto_blacklist")) {
            Logger.dev("SplashActivity: Setting default preferences for application");
            SharedPreferences.Editor editor = prefs.edit();
            set.add("com.google.android.apps.walletnfcrel");
            set.add("com.google.android.gms");
            set.add("com.google.commerce.tapandpay");
            editor.putStringSet("auto_blacklist", set);
            editor.putBoolean("autoRootEnable", false);
            editor.putBoolean("root", Utils.rootEnabled());
            editor.apply();
        }


        // Initialize
        prefs.edit()
                .putBoolean("module_done", false)
                .putBoolean("repo_done", false)
                .putBoolean("update_check_done", false)
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
