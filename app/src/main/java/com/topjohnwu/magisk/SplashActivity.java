package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.GetBootBlocks;
import com.topjohnwu.magisk.asyncs.LoadApps;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.LoadRepos;
import com.topjohnwu.magisk.asyncs.MagiskHide;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        MagiskManager magiskManager = getTopApplication();

        // Init the info and configs and root shell
        magiskManager.init();

        // Check MagiskHide status
        List<String> ret = Shell.sh("getprop persist.magisk.hide");
        boolean started = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;

        // Now fire all async tasks
        new CheckUpdates(this).exec();
        new GetBootBlocks(this).exec();
        if (magiskManager.magiskHide && !magiskManager.disabled &&
                magiskManager.magiskVersion > 11 && !started) {
            new MagiskHide().enable();
        }
        new LoadModules(this) {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new LoadRepos(activity).exec();
            }
        }.exec();
        new LoadApps(this).exec();

        // Preparation done, now start main activity
        Intent intent = new Intent(getApplicationContext(), MainActivity.class);
        startActivity(intent);
        finish();
    }
}
