package com.topjohnwu.magisk.utils;

import android.util.Log;

public class Logger {

    public static final String TAG = "Magisk";
    public static final String DEV_TAG = "Magisk: DEV";
    public static final String DEBUG_TAG = "Magisk: DEBUG";

    public static boolean logShell, devLog;

    public static void debug(String msg) {
        Log.d(DEBUG_TAG, msg);
    }

    public static void dev(String msg, Object... args) {
        if (devLog) {
            if (args.length == 1 && args[0] instanceof Throwable) {
                Log.d(DEV_TAG, msg, (Throwable) args[0]);
            } else {
                Log.d(DEV_TAG, String.format(msg, args));
            }
        }
    }

    public static void dev(String msg) {
        if (devLog) {
            Log.d(DEV_TAG, msg);
        }
    }

    public static void shell(boolean root, String msg) {
        if (logShell) {
            Log.d(root ? "SU" : "SH", msg);
        }
    }
}
