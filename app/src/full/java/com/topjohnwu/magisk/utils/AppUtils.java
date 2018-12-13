package com.topjohnwu.magisk.utils;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.services.UpdateCheckService;

public class AppUtils {

    public static void setupUpdateCheck() {
        App app = App.self;
        JobScheduler scheduler = (JobScheduler) app.getSystemService(Context.JOB_SCHEDULER_SERVICE);

        if (app.prefs.getBoolean(Const.Key.CHECK_UPDATES, true)) {
            if (scheduler.getAllPendingJobs().isEmpty() ||
                    Const.UPDATE_SERVICE_VER > app.prefs.getInt(Const.Key.UPDATE_SERVICE_VER, -1)) {
                ComponentName service = new ComponentName(app, ClassMap.get(UpdateCheckService.class));
                JobInfo info = new JobInfo.Builder(Const.ID.UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                scheduler.schedule(info);
            }
        } else {
            scheduler.cancel(Const.UPDATE_SERVICE_VER);
        }
    }

    public static void openLink(Context context, Uri link) {
        Intent intent = new Intent(Intent.ACTION_VIEW, link);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (intent.resolveActivity(context.getPackageManager()) != null) {
            context.startActivity(intent);
        } else {
            Utils.toast(R.string.open_link_failed_toast, Toast.LENGTH_SHORT);
        }
    }

}