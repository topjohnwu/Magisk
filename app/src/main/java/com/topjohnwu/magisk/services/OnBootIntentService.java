package com.topjohnwu.magisk.services;

import android.app.IntentService;
import android.content.Intent;
import android.os.Build;
import android.support.v4.app.NotificationCompat;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Const;

public class OnBootIntentService extends IntentService {

    public OnBootIntentService() {
        super("OnBootIntentService");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationCompat.Builder builder =
                    new NotificationCompat.Builder(this, Const.ID.NOTIFICATION_CHANNEL);
            builder.setSmallIcon(R.drawable.ic_magisk)
                    .setContentTitle("onBoot")
                    .setContentText("Running onBoot operations...");
            startForeground(Const.ID.ONBOOT_NOTIFICATION_ID, builder.build());
        }
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        // Currently nothing to do
    }
}
