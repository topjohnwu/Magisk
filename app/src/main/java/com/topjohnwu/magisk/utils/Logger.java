package com.topjohnwu.magisk.utils;

import android.app.Application;
import android.content.Context;
import android.preference.PreferenceManager;
import android.util.Log;

public class Logger {

    private static final String LOG_TAG = "Magisk: DEV";

    public static void dev(String msg, Object... args) {
        Context context = null;
        try {
            context = getApplicationUsingReflection();
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("developer_logging", false)) {
            if (args.length == 1 && args[0] instanceof Throwable) {
                Log.d(LOG_TAG, msg, (Throwable) args[0]);
            } else {
                Log.d(LOG_TAG, String.format(msg, args));
            }
        }
    }

    public static void dev(String msg) {
        Context context = null;
        try {
            context = getApplicationUsingReflection();
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("developer_logging", false)) {
            Log.d(LOG_TAG, msg);
        }
    }

    private static Application getApplicationUsingReflection() throws Exception {
        return (Application) Class.forName("android.app.AppGlobals")
                .getMethod("getInitialApplication").invoke(null, (Object[]) null);
    }
}
