package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.topjohnwu.magisk.services.TileServiceCompat;

public class TileReceiver extends BroadcastReceiver {
    private static final String TAG = "MainReceiver";
    public TileReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG,"RECEIVED");
        String action = intent.getAction();

        if (action.equals(Intent.ACTION_BOOT_COMPLETED) || action.equals(Intent.ACTION_USER_PRESENT) || action.equals(Intent.ACTION_SCREEN_ON)) {
            context.startService(new Intent(context,TileServiceCompat.class));

        }
    }
}
