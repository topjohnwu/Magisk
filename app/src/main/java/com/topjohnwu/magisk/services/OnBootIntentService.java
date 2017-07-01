package com.topjohnwu.magisk.services;

import android.app.IntentService;
import android.content.Intent;
import android.os.Build;
import android.support.v7.app.NotificationCompat;

import com.topjohnwu.magisk.R;

public class OnBootIntentService extends IntentService {

    private static final int ONBOOT_NOTIFICATION_ID = 3;

    public OnBootIntentService() {
        super("OnBootIntentService");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationCompat.Builder builder = new NotificationCompat.Builder(this);
            builder.setSmallIcon(R.drawable.ic_magisk)
                    .setContentTitle("onBoot")
                    .setContentText("Running onBoot operations...");
            startForeground(ONBOOT_NOTIFICATION_ID, builder.build());
        }
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        // Currently nothing to do
    }
}
