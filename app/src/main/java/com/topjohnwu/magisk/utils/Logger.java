package com.topjohnwu.magisk.utils;

import android.util.Log;

public class Logger {

    private static final String LOG_TAG = "Magisk: DEV";

    public static boolean logShell, devLog;

    public static void dev(String msg, Object... args) {
        if (devLog) {
            if (args.length == 1 && args[0] instanceof Throwable) {
                Log.d(LOG_TAG, msg, (Throwable) args[0]);
            } else {
                Log.d(LOG_TAG, String.format(msg, args));
            }
        }
    }

    public static void dev(String msg) {
        if (devLog) {
            Log.d(LOG_TAG, msg);
        }
    }

    public static void shell(boolean root, String msg) {
        if (logShell) {
            Log.d(root ? "SU" : "SH", msg);
        }
    }
}
