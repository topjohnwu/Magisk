package com.topjohnwu.magisk.tile;

import android.content.*;
import android.widget.Toast;

/**
 * Exported receiver for the custom event on the custom Quick Settings tile
 */
public final class PublicBroadcastReceiver extends BroadcastReceiver {
    /**
     * The action broadcast from the Quick Settings tile when clicked
     */
    public static final String ACTION_TOAST = "com.kcoppock.CUSTOMTILE_ACTION_TOAST";

    /**
     * Constant for the String extra to be displayed in the Toast
     */
    public static final String EXTRA_MESSAGE = "com.kcoppock.CUSTOMTILE_EXTRA_MESSAGE";

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();

        if (ACTION_TOAST.equals(action)) {
            final String message = intent.getStringExtra(EXTRA_MESSAGE);
            Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
        }
    }
}