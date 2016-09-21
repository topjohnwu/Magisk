package com.topjohnwu.magisk.utils;

import android.util.Log;



public class Logger {

    private static final String LOG_TAG = "Magisk";

    public static void dh(String msg, Object... args) {
        if (PrefHelper.CheckBool("developer_logging")) {
            if (args.length == 1 && args[0] instanceof Throwable) {
                Log.d(LOG_TAG, msg, (Throwable) args[0]);
            } else {
                Log.d(LOG_TAG, String.format(msg, args));
            }
        }
    }
}
