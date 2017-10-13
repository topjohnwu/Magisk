package com.topjohnwu.magisk.utils;

import android.util.Log;

import com.topjohnwu.magisk.MagiskManager;

import java.util.Locale;

public class Logger {

    public static final String DEBUG_TAG = "MagiskManager";
    private static final boolean SHELL_LOGGING = false;

    public static void debug(String line) {
        Log.d(DEBUG_TAG, "DEBUG: " + line);
    }

    public static void debug(String fmt, Object... args) {
        debug(String.format(Locale.US, fmt, args));
    }

    public static void error(String line) {
        Log.e(DEBUG_TAG, "ERROR: " + line);
    }

    public static void error(String fmt, Object... args) {
        error(String.format(Locale.US, fmt, args));
    }

    public static void shell(String line) {
        if (SHELL_LOGGING) {
            Log.d(DEBUG_TAG, "SHELL: " + line);
        }
    }

    public static void shell(String fmt, Object... args) {
        shell(String.format(Locale.US, fmt, args));
    }
}
