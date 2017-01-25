package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Utils;

public class SplashActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

        Global.Configs.init(getApplicationContext());

        if (Global.Configs.isDarkTheme) {
            setTheme(R.style.AppTheme_dh);
        }

        // Initialize prefs
        prefs.edit()
                .putBoolean("magiskhide", Utils.itemExist(false, "/magisk/.core/magiskhide/enable"))
                .putBoolean("busybox", Utils.commandExists("busybox"))
                .putBoolean("hosts", Utils.itemExist(false, "/magisk/.core/hosts"))
                .apply();

        // Start all async tasks
        new Async.GetBootBlocks().exec();
        new Async.CheckUpdates().exec();
        new Async.LoadModules() {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(getApplicationContext()).exec();
            }
        }.exec();
        new Async.LoadApps(getPackageManager()).exec();

        // Start main activity
        Intent intent = new Intent(getApplicationContext(), MainActivity.class);
        startActivity(intent);
        finish();
    }
}
