package com.topjohnwu.magisk.utils;


import android.app.Application;
import android.content.Context;
import android.preference.PreferenceManager;

public class PrefHelper {
    public PrefHelper() {

    }

    public static boolean CheckBool(String key) {
        Context context = null;
        try {
            context = getApplicationUsingReflection();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean(key, false);
    }

    private static Application getApplicationUsingReflection() throws Exception {
        return (Application) Class.forName("android.app.AppGlobals")
                .getMethod("getInitialApplication").invoke(null, (Object[]) null);
    }

}
