package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.utils.Async;

public class SplashActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        // Init the info and configs and root shell
        Global.init(getApplicationContext());

        // Start MagiskHide if not started at boot
        if (Global.Configs.magiskHide && !Global.Info.disabled && Global.Info.magiskVersion > 10.3)
            new Async.MagiskHide().enable();

        // Now fire all async tasks
        new Async.LoadApps(getPackageManager()).exec();
        new Async.GetBootBlocks().exec();
        new Async.CheckUpdates().exec();
        new Async.LoadModules() {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(getApplicationContext()).exec();
            }
        }.exec();

        // Preparation done, now start main activity
        Intent intent = new Intent(getApplicationContext(), MainActivity.class);
        startActivity(intent);
        finish();
    }
}
