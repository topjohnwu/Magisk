package com.topjohnwu.magisk;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AutoStart extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("Magisk", "Received Boot call, attempting to start service");
        Intent myIntent = new Intent(context, MonitorService.class);
        context.startService(myIntent);

    }
}