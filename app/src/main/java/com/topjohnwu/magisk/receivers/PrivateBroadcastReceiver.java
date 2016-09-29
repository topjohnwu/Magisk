package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.topjohnwu.magisk.utils.Utils;

/**
 * Receiver for click events on the custom M Quick Settings tile
 */
public final class PrivateBroadcastReceiver extends BroadcastReceiver {
    public static final String ACTION_AUTOROOT = "com.topjohnwu.magisk.CUSTOMTILE_ACTION_AUTOROOT";
    public static final String ACTION_DISABLEROOT = "com.topjohnwu.magisk.CUSTOMTILE_ACTION_DISABLEROOT";
    public static final String ACTION_ENABLEROOT = "com.topjohnwu.magisk.CUSTOMTILE_ACTION_ENABLEROOT";


    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        Log.d("Magisk","Broadcast Receiver, Made it this far!  We're trying to " + action);
        if (ACTION_AUTOROOT.equals(action)) {
            Utils.toggleAutoRoot(!Utils.autoToggleEnabled(context),context);
        }
        if (ACTION_ENABLEROOT.equals(action)) {
            Utils.toggleAutoRoot(false, context);
            Utils.toggleRoot(true, context);
        }
        if (ACTION_DISABLEROOT.equals(action)) {
            Utils.toggleAutoRoot(false, context);
            Utils.toggleRoot(false, context);
        }

        Utils.setupQuickSettingsTile(context);
    }
}