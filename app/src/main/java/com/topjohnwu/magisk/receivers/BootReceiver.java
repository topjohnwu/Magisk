package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Async;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager magiskManager = (MagiskManager) context.getApplicationContext();
        magiskManager.initSuAccess();
        magiskManager.updateMagiskInfo();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (prefs.getBoolean("magiskhide", false) && !magiskManager.disabled && magiskManager.magiskVersion > 10.3) {
            Toast.makeText(context, R.string.start_magiskhide, Toast.LENGTH_SHORT).show();
            new Async.MagiskHide(true).enable();
        }
    }
}
