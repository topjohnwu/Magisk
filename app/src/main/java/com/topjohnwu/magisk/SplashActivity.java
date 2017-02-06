package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Utils;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        MagiskManager magiskManager = getTopApplication();

        // Init the info and configs and root shell
        magiskManager.init();
        boolean boot_done = Utils.itemExist(MagiskManager.MAGISK_MANAGER_BOOT);

        // Now fire all async tasks
        new Async.CheckUpdates(magiskManager).exec();
        new Async.GetBootBlocks(magiskManager).exec();
        if (magiskManager.prefs.getBoolean("magiskhide", false) && !magiskManager.disabled &&
                magiskManager.magiskVersion > 10.3 && boot_done) {
            new Async.MagiskHide().enable();
        }
        new Async.LoadModules(magiskManager) {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(magiskManager).exec();
            }
        }.exec();
        new Async.LoadApps(magiskManager).exec();

        // Preparation done, now start main activity
        Intent intent = new Intent(getApplicationContext(), MainActivity.class);
        startActivity(intent);
        finish();
    }
}
