package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.services.BootupIntentService;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        context.startService(new Intent(context, BootupIntentService.class));
    }

}
