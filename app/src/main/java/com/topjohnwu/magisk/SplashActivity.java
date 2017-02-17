package com.topjohnwu.magisk;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.GetBootBlocks;
import com.topjohnwu.magisk.asyncs.LoadApps;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.LoadRepos;
import com.topjohnwu.magisk.asyncs.MagiskHide;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class SplashActivity extends Activity{

    private static final int UPDATE_SERVICE_ID = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        MagiskManager magiskManager = getApplicationContext();

        // Init the info and configs and root shell
        magiskManager.init();

        // Check MagiskHide status
        List<String> ret = Shell.sh("getprop persist.magisk.hide");
        boolean started = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;

        // Initialize the update check service, notify every 3 hours
        if (!"install".equals(getIntent().getStringExtra(MainActivity.SECTION))) {
            ComponentName service = new ComponentName(magiskManager, UpdateCheckService.class);
            JobInfo jobInfo = new JobInfo.Builder(UPDATE_SERVICE_ID, service)
                    .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                    .setPersisted(true)
                    .setPeriodic(3 * 60 * 60 * 1000)
                    .build();
            JobScheduler scheduler = (JobScheduler) getSystemService(Context.JOB_SCHEDULER_SERVICE);
            scheduler.schedule(jobInfo);
        }

        // Now fire all async tasks
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
        new CheckUpdates(this, !"install".equals(getIntent().getStringExtra(MainActivity.SECTION))){
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                Intent intent = getIntent().setClass(magiskManager, MainActivity.class)
                        .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
        }.exec();
    }
}
