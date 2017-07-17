package com.topjohnwu.magisk.utils;

import android.util.Log;

import com.topjohnwu.magisk.MagiskManager;

import java.util.Locale;

public class Logger {

    public static final String MAIN_TAG = "Magisk";
    public static final String DEBUG_TAG = "MagiskManager";

    public static void debug(String line) {
        Log.d(DEBUG_TAG, "DEBUG: " + line);
    }

    public static void debug(String fmt, Object... args) {
        debug(String.format(Locale.US, fmt, args));
    }

    public static void error(String line) {
        Log.e(MAIN_TAG, "MANAGERERROR: " + line);
    }

    public static void error(String fmt, Object... args) {
        error(String.format(Locale.US, fmt, args));
    }

    public static void dev(String line) {
        if (MagiskManager.devLogging) {
            Log.d(DEBUG_TAG, line);
        }
    }

    public static void dev(String fmt, Object... args) {
        dev(String.format(Locale.US, fmt, args));
    }

    public static void shell(String line) {
        if (MagiskManager.shellLogging) {
            Log.d(DEBUG_TAG, "SHELL: " + line);
        }
    }

    public static void shell(String fmt, Object... args) {
        shell(String.format(Locale.US, fmt, args));
    }
}
