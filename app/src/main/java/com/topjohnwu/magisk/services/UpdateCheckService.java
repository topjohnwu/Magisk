package com.topjohnwu.magisk.services;

import android.app.job.JobParameters;
import android.app.job.JobService;

import com.topjohnwu.magisk.asyncs.CheckUpdates;

public class UpdateCheckService extends JobService {
    @Override
    public boolean onStartJob(JobParameters params) {
        new CheckUpdates(this, true){
            @Override
            protected Void doInBackground(Void... voids) {
                magiskManager.updateMagiskInfo();
                return super.doInBackground(voids);
            }

            @Override
            protected void onPostExecute(Void v) {
                jobFinished(params, false);
                super.onPostExecute(v);
            }
        }.exec();
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }
}
