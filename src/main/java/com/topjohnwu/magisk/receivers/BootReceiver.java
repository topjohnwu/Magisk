package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import com.topjohnwu.magisk.services.OnBootIntentService;

public class BootReceiver extends BroadcastReceiver {

    private void startIntentService(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(new Intent(context, OnBootIntentService.class));
        } else {
            context.startService(new Intent(context, OnBootIntentService.class));
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        startIntentService(context);
    }

}
