package com.topjohnwu.magisk;

import static android.os.Build.VERSION.SDK_INT;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.File;
import java.lang.reflect.Method;
import java.util.Map;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class DynAPK {
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

    public static void addAssetPath(AssetManager asset, String path) {
        try {
            if (addAssetPath == null)
                addAssetPath = AssetManager.class.getMethod("addAssetPath", String.class);
            addAssetPath.invoke(asset, path);
        } catch (Exception ignored) {}
    }

    public static class Data {
        // Indices of the object array
        private static final int STUB_VERSION = 0;
        private static final int CLASS_COMPONENT_MAP = 1;
        private static final int ROOT_SERVICE = 2;
        private static final int ARR_SIZE = 3;

        private final Object[] arr;

        public Data() { arr = new Object[ARR_SIZE]; }
        public Data(Object o) { arr = (Object[]) o; }
        public Object getObject() { return arr; }

        public int getVersion() { return (int) arr[STUB_VERSION]; }
        public void setVersion(int version) { arr[STUB_VERSION] = version; }
        public Map<String, String> getClassToComponent() {
            // noinspection unchecked
            return (Map<String, String>) arr[CLASS_COMPONENT_MAP];
        }
        public void setClassToComponent(Map<String, String> map) {
            arr[CLASS_COMPONENT_MAP] = map;
        }
        public Class<?> getRootService() { return (Class<?>) arr[ROOT_SERVICE]; }
        public void setRootService(Class<?> service) { arr[ROOT_SERVICE] = service; }
    }
}
