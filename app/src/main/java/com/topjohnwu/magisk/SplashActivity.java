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
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.HashSet;
import java.util.Set;

public class SplashActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString("theme","").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }
        super.onCreate(savedInstanceState);

        //setups go here

        // Set up default preferences,make sure we add "extra" blacklist entries.

        PreferenceManager.setDefaultValues(this, R.xml.defaultpref, false);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplication());

        if (!prefs.contains("auto_blacklist")) {
            Logger.dh("AutoRootFragment: Setting default preferences for application");
            SharedPreferences.Editor editor = prefs.edit();
            Set<String> set = new HashSet<>();
            set.add("com.google.android.apps.walletnfcrel");
            set.add("com.google.android.gms");
            set.add("com.google.commerce.tapandpay");
            editor.putStringSet("auto_blacklist", set);
            editor.apply();
        }

        // Set up toggle states based on preferences, start services, disable root if set
        if (Utils.autoToggleEnabled(getApplicationContext())) {
            if (!Utils.hasServicePermission(getApplicationContext())) {
                Utils.toggleAutoRoot(false,getApplicationContext());
            }
        }
        if (Utils.autoToggleEnabled(getApplicationContext())) {

            if (!Utils.isMyServiceRunning(MonitorService.class, getApplicationContext())) {
                Intent myIntent = new Intent(getApplication(), MonitorService.class);
                getApplication().startService(myIntent);
            }
        } else {
            if (PreferenceManager.getDefaultSharedPreferences(getApplication()).getBoolean("keep_root_off", false)) {
                Utils.toggleRoot(false, getApplication());
            }
        }

        // Set up quick settings tile
        Utils.SetupQuickSettingsTile(getApplicationContext());

        // Initialize


            if (Shell.rootAccess()) {
                if (!Utils.busyboxInstalled()) {
                    String busybox = getApplicationContext().getApplicationInfo().nativeLibraryDir + "/libbusybox.so";
                    Shell.su(
                            "rm -rf /data/busybox",
                            "mkdir -p /data/busybox",
                            "cp -af " + busybox + " /data/busybox/busybox",
                            "chmod 755 /data/busybox /data/busybox/busybox",
                            "chcon u:object_r:system_file:s0 /data/busybox /data/busybox/busybox",
                            "/data/busybox/busybox --install -s /data/busybox",
                            "rm -f /data/busybox/su",
                            "export PATH=/data/busybox:$PATH"
                    );
                }
            }

        new Async.CheckUpdates(this).execute();
        new Async.LoadModules(this).execute();
        new Async.LoadRepos(this).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);


        // Start main activity
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);

        finish();
    }
}
