package com.topjohnwu.magisk;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;

import com.topjohnwu.magisk.asyncs.LoadApps;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.utils.Utils;

public class SplashActivity extends Activity{

    private static final int UPDATE_SERVICE_ID = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        MagiskManager magiskManager = getApplicationContext();

        // Init the info and configs and root sh
        magiskManager.init();

        // Get possible additional info from intent
        magiskManager.remoteMagiskVersionString = getIntent().getStringExtra(MagiskManager.INTENT_VERSION);
        magiskManager.magiskLink = getIntent().getStringExtra(MagiskManager.INTENT_LINK);


        LoadModules loadModuleTask = new LoadModules(this);

        if (Utils.checkNetworkStatus(this)) {
            // Initialize the update check service, notify every 8 hours
            if (!TextUtils.equals("install", getIntent().getStringExtra(MagiskManager.INTENT_SECTION))) {
                ComponentName service = new ComponentName(this, UpdateCheckService.class);
                JobInfo jobInfo = new JobInfo.Builder(UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                JobScheduler scheduler = (JobScheduler) getSystemService(Context.JOB_SCHEDULER_SERVICE);
                scheduler.schedule(jobInfo);
            }
            loadModuleTask.setCallBack(() -> new UpdateRepos(this).exec());
        }

        // Now fire all async tasks
        loadModuleTask.exec();
        new LoadApps(this).exec();

        Intent intent = new Intent(this, MainActivity.class);
        String section = getIntent().getStringExtra(MagiskManager.INTENT_SECTION);
        if (section != null) {
            intent.putExtra(MagiskManager.INTENT_SECTION, section);
        }
        startActivity(intent);
        finish();
    }
}
