package com.topjohnwu.magisk.utils;

import android.content.Context;

import java.io.File;
import java.util.Map;

public class DynAPK {

    // Indices of the object array
    private static final int COMPONENT_MAP = 0;
    private static final int RESOURCE_MAP = 1;

    // Indices of the resource map
    public static final int NOTIFICATION = 0;
    public static final int DOWNLOAD = 1;
    public static final int SUPERUSER = 2;
    public static final int MODULES = 3;
    public static final int MAGISKHIDE = 4;

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

    public static Data load(Object o) {
        Object[] arr = (Object[]) o;
        Data data = new Data();
        data.componentMap = (Map<String, String>) arr[COMPONENT_MAP];
        data.resourceMap = (int[]) arr[RESOURCE_MAP];
        return data;
    }

    public static Object pack(Data data) {
        Object[] arr = new Object[2];
        arr[COMPONENT_MAP] = data.componentMap;
        arr[RESOURCE_MAP] = data.resourceMap;
        return arr;
    }

    public static class Data {
        public Map<String, String> componentMap;
        public int[] resourceMap;
    }
}
