package com.topjohnwu.magisk;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.File;
import java.lang.reflect.Method;
import java.util.Map;

import static android.os.Build.VERSION.SDK_INT;

public class DynAPK {

    private static final int STUB_VERSION = 4;

    // Indices of the object array
    private static final int STUB_VERSION_ENTRY = 0;
    private static final int COMPONENT_MAP = 1;

    private static File dynDir;
    private static Method addAssetPath;

    private static File getDynDir(Context c) {
        if (dynDir == null) {
            if (SDK_INT >= 24) {
                // Use protected context to allow directBootAware
                c = c.createDeviceProtectedStorageContext();
            }
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

    public static Data load(Object o) {
        Object[] arr = (Object[]) o;
        Data data = new Data();
        data.version = (int) arr[STUB_VERSION_ENTRY];
        data.componentMap = (Map<String, String>) arr[COMPONENT_MAP];
        return data;
    }

    public static Object pack(Data data) {
        Object[] arr = new Object[2];
        arr[STUB_VERSION_ENTRY] = data.version;
        arr[COMPONENT_MAP] = data.componentMap;
        return arr;
    }

    public static void addAssetPath(AssetManager asset, String path) {
        try {
            if (addAssetPath == null)
                addAssetPath = AssetManager.class.getMethod("addAssetPath", String.class);
            addAssetPath.invoke(asset, path);
        } catch (Exception ignored) {}
    }

    public static class Data {
        public int version = STUB_VERSION;
        public Map<String, String> componentMap;
    }
}
