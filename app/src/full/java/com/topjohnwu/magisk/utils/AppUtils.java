package com.topjohnwu.magisk.utils;

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

import java.util.concurrent.TimeUnit;

import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.PeriodicWorkRequest;
import androidx.work.WorkManager;

public class AppUtils {

    public static void scheduleUpdateCheck() {
        if (App.self.prefs.getBoolean(Const.Key.CHECK_UPDATES, true)) {
            Constraints constraints = new Constraints.Builder()
                    .setRequiredNetworkType(NetworkType.CONNECTED)
                    .build();
            PeriodicWorkRequest request = new PeriodicWorkRequest
                    .Builder(ClassMap.get(UpdateCheckService.class), 12, TimeUnit.HOURS)
                    .setConstraints(constraints)
                    .build();
            WorkManager.getInstance().enqueueUniquePeriodicWork(Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID,
                    ExistingPeriodicWorkPolicy.REPLACE, request);
        } else {
            WorkManager.getInstance().cancelUniqueWork(Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID);
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