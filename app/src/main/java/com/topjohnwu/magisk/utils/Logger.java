package com.topjohnwu.magisk.utils;

import android.util.Log;

import com.topjohnwu.magisk.MagiskManager;

import java.util.Locale;

public class Logger {

    public static final String MAIN_TAG = "Magisk";
    public static final String DEBUG_TAG = "MagiskManager";

    public static void debug(String fmt, Object... args) {
        Log.d(DEBUG_TAG, "DEBUG: " + String.format(Locale.US, fmt, args));
    }

    public static void error(String fmt, Object... args) {
        Log.e(MAIN_TAG, "MANAGERERROR: " + String.format(Locale.US, fmt, args));
    }

    public static void dev(String fmt, Object... args) {
        if (MagiskManager.devLogging) {
            Log.d(DEBUG_TAG, String.format(Locale.US, fmt, args));
        }
    }

    public static void shell(boolean root, String fmt, Object... args) {
        if (MagiskManager.shellLogging) {
            Log.d(DEBUG_TAG, (root ? "MANAGERSU: " : "MANAGERSH: ") + String.format(Locale.US, fmt, args));
        }
    }
}
