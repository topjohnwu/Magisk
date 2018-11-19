package com.topjohnwu.magisk.services;

import android.app.job.JobParameters;
import android.app.job.JobService;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.superuser.Shell;

public class UpdateCheckService extends JobService {

    @Override
    public boolean onStartJob(JobParameters params) {
        Shell.getShell();
        CheckUpdates.check(() -> jobFinished(params, false));
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }
}
