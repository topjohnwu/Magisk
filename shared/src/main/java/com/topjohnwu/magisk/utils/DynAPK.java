package com.topjohnwu.magisk.utils;

import android.content.Context;

import java.io.File;

public class DynAPK {

    private static File dynDir;

    private static File getDynDir(Context c) {
        if (dynDir == null) {
            dynDir = new File(c.getFilesDir().getParent(), "dyn");
            dynDir.mkdir();
        }
        return dynDir;
    }

    public static File current(Context c) {
        return new File(getDynDir(c), "current.apk");
    }

    public static File update(Context c) {
        return new File(getDynDir(c), "update.apk");
    }
}
