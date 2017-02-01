package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.topjohnwu.magisk.Global;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Async;

public class BootReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        Global.initSuAccess();
        if (prefs.getBoolean("magiskhide", false)) {
            Toast.makeText(context, R.string.start_magiskhide, Toast.LENGTH_SHORT).show();
            new Async.MagiskHide().enable();
        }
    }
}
