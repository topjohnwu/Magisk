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

public class SplashActivity extends Activity{

    private static final int UPDATE_SERVICE_ID = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        MagiskManager magiskManager = getApplicationContext();

        // Init the info and configs and root shell
        magiskManager.init();

        // Initialize the update check service, notify every 3 hours
        if (!"install".equals(getIntent().getStringExtra(MagiskManager.INTENT_SECTION))) {
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
                magiskManager.magiskVersion > 11 && !magiskManager.magiskHideStarted) {
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
        new CheckUpdates(this, false){
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                String section = getIntent().getStringExtra(MagiskManager.INTENT_SECTION);
                Intent intent = new Intent(magiskManager, MainActivity.class);
                if (section != null) {
                    intent.putExtra(MagiskManager.INTENT_SECTION, section);
                }
                startActivity(intent);
                finish();
            }
        }.exec();
    }
}
