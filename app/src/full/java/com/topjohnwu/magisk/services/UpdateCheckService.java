package com.topjohnwu.magisk.services;

import android.app.job.JobParameters;
import android.app.job.JobService;

import com.topjohnwu.core.Data;
import com.topjohnwu.core.tasks.CheckUpdates;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.superuser.Shell;

public class UpdateCheckService extends JobService {

    @Override
    public boolean onStartJob(JobParameters params) {
        Shell.getShell();
        CheckUpdates.check(() -> {
            if (BuildConfig.VERSION_CODE < Data.remoteManagerVersionCode) {
                Notifications.managerUpdate();
            } else if (Data.magiskVersionCode < Data.remoteMagiskVersionCode) {
                Notifications.magiskUpdate();
            }
            jobFinished(params, false);
        });
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }
}
