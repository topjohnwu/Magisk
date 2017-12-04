package com.topjohnwu.magisk.utils;

import android.util.Log;

import java.util.Locale;

public class Logger {

    private static final boolean SHELL_LOGGING = false;

    public static void debug(String line) {
        Log.d(Const.DEBUG_TAG, "DEBUG: " + line);
    }

    public static void debug(String fmt, Object... args) {
        debug(String.format(Locale.US, fmt, args));
    }

    public static void error(String line) {
        Log.e(Const.DEBUG_TAG, "ERROR: " + line);
    }

    public static void error(String fmt, Object... args) {
        error(String.format(Locale.US, fmt, args));
    }

    public static void shell(boolean in, String line) {
        if (SHELL_LOGGING) {
            Log.d(Const.DEBUG_TAG, (in ? "SHELLIN : " : "SHELLOUT: ") + line);
        }
    }

    public static void shell(boolean in, String fmt, Object... args) {
        shell(in, String.format(Locale.US, fmt, args));
    }
}
