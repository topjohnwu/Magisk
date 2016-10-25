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

import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class SplashActivity extends AppCompatActivity {
private SharedPreferences prefs;
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        prefs = PreferenceManager.getDefaultSharedPreferences(getApplication());
        if (prefs.getString("theme", "").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }

        Logger.devLog = prefs.getBoolean("developer_logging", false);
        Logger.logShell = prefs.getBoolean("shell_logging", false);

        // Check and set preferences/hides
        setupHideLists();

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

    private void setupHideLists() {

        Set<String> set = null;
        Set<String> setOriginal = null;
        List<String> hideList = null;
        List<String> addList = null;
        String listCmd, addCmd, addCmd2, rmCmd, rmCmd2;

        // Build list of apps currently listed, add to preferences

        int hideVersion = Utils.WhichHide(getApplication());

        switch (hideVersion) {
            case 1:
                listCmd = "/magisk/.core/magiskhide/list";
                break;
            case 2:
                listCmd = "/su/suhide/list";
                break;
            case 3:
                listCmd = "/magisk/.core/magiskhide/list && /su/suhide/list";
                break;
            default:
                listCmd = "";

        }
        if (Shell.rootAccess()) {
            hideList = Shell.su(listCmd);
        }
        // Set up default preferences,make sure we add "extra" blacklist entries.

        if (!prefs.contains("auto_blacklist")) {
            Logger.dev("SplashActivity: Setting default preferences for application");
            set.add("com.google.android.apps.walletnfcrel");
            set.add("com.google.android.gms");
            set.add("com.google.commerce.tapandpay");

            // Add current items to hide list
            if (hideList != null) set.addAll(hideList);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putStringSet("auto_blacklist", set);
            Logger.dev("SplashActivity: Adding entries " + set.toString());
            editor.apply();
        }

        setOriginal = prefs.getStringSet("auto_blacklist", set);
        if (hideList != null) {
            for (String item : hideList) {
                if (!(setOriginal.contains(item))) {
                    addList.add(item);
                }
            }
        }

        SharedPreferences.Editor editor = prefs.edit();
        editor.putStringSet("auto_blacklist", set);
        editor.apply();

    }
}
