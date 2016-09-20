package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.topjohnwu.magisk.services.MonitorService;
import com.topjohnwu.magisk.utils.Utils;

public class AutoStartReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("Magisk", "Received Boot call, attempting to start service");
        Intent myIntent = new Intent(context, MonitorService.class);
        context.startService(myIntent);
        Utils.SetupQuickSettingsTile(context);

    }
}