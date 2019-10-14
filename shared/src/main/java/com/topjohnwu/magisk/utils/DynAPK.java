package com.topjohnwu.magisk.utils;

import android.content.Context;

import java.io.File;

public class DynAPK {

    private static File dynDir;

    public static File current(Context context) {
        if (dynDir == null)
            dynDir = new File(context.getFilesDir().getParent(), "dyn");
        return new File(dynDir, "current.apk");
    }
}
