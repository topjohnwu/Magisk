package com.topjohnwu.magisk;

import static com.topjohnwu.magisk.BuildConfig.APPLICATION_ID;

import android.app.AppComponentFactory;
import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import com.topjohnwu.magisk.utils.APKInstall;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashMap;

@SuppressWarnings("ResultOfMethodCallIgnored")
public class DynLoad {

    static Object componentFactory;
    static ClassLoader activeClassLoader = DynLoad.class.getClassLoader();

    static StubApk.Data createApkData() {
        var data = new StubApk.Data();
        data.setVersion(BuildConfig.STUB_VERSION);
        data.setClassToComponent(new HashMap<>());
        data.setRootService(DelegateRootService.class);
        return data;
    }

    static void attachContext(Object o, Context context) {
        if (!(o instanceof ContextWrapper))
            return;
        try {
            Method m = ContextWrapper.class.getDeclaredMethod("attachBaseContext", Context.class);
            m.setAccessible(true);
            m.invoke(o, context);
        } catch (Exception ignored) { /* Impossible */ }
    }

    // Dynamically load APK from internal, external storage, or previous app
    static AppClassLoader loadApk(Context context) {
        File apk = StubApk.current(context);
        File update = StubApk.update(context);

        if (update.exists()) {
            // Rename from update
            update.renameTo(apk);
        }

        // Copy from external for easier development
        if (BuildConfig.DEBUG) {
            try {
                File external = new File(context.getExternalFilesDir(null), "magisk.apk");
                if (external.exists()) {
                    apk.delete();
                    try {
                        var in = new FileInputStream(external);
                        var out = new FileOutputStream(apk);
                        apk.setReadOnly();
                        try (in; out) {
                            APKInstall.transfer(in, out);
                        }
                    } catch (IOException e) {
                        Log.e(DynLoad.class.getSimpleName(), "", e);
                        apk.delete();
                    } finally {
                        external.delete();
                    }
                }
            } catch (SecurityException e) {
                // Do not crash in root service
            }
        }

        if (apk.exists()) {
            apk.setReadOnly();
            return new AppClassLoader(apk);
        }

        // If no APK is loaded, attempt to copy from previous app
        if (!context.getPackageName().equals(APPLICATION_ID)) {
            try {
                var info = context.getPackageManager().getApplicationInfo(APPLICATION_ID, 0);
                apk.delete();
                var src = new FileInputStream(info.sourceDir);
                var out = new FileOutputStream(apk);
                apk.setReadOnly();
                try (src; out) {
                    APKInstall.transfer(src, out);
                }
                return new AppClassLoader(apk);
            } catch (PackageManager.NameNotFoundException ignored) {
            } catch (IOException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }
        }

        return null;
    }

    // Dynamically load APK and create the Application instance from the loaded APK
    static Application createAndSetupApp(Application context) {
        // On API >= 29, AppComponentFactory will replace the ClassLoader for us
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q)
            replaceClassLoader(context);

        // noinspection InlinedApi
        int flags = PackageManager.GET_ACTIVITIES | PackageManager.GET_SERVICES
                | PackageManager.GET_PROVIDERS | PackageManager.GET_RECEIVERS
                | PackageManager.MATCH_DIRECT_BOOT_AWARE | PackageManager.MATCH_DIRECT_BOOT_UNAWARE;
        var pm = context.getPackageManager();

        final PackageInfo info;
        try {
            // noinspection WrongConstant
            info = pm.getPackageInfo(context.getPackageName(), flags);
        } catch (PackageManager.NameNotFoundException e) {
            // Impossible
            throw new RuntimeException(e);
        }

        File apk = StubApk.current(context);

        final var cl = loadApk(context);
        if (cl != null) try {
            // noinspection WrongConstant
            var pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), flags);
            cl.updateComponentMap(info, pkgInfo);

            var appInfo = pkgInfo.applicationInfo;

            var data = createApkData();
            var map = data.getClassToComponent();
            // Create the inverse mapping (class to component name)
            for (var e : cl.mapping.entrySet()) {
                map.put(e.getValue(), e.getKey());
            }

            // Create the receiver Application
            var app = (Application) cl.loadClass(appInfo.className)
                    .getConstructor(Object.class)
                    .newInstance(data.getObject());

            // Create the receiver component factory
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P && componentFactory != null) {
                var delegate = (DelegateComponentFactory) componentFactory;
                if (appInfo.appComponentFactory == null) {
                    delegate.receiver = new AppComponentFactory();
                } else {
                    Object factory = cl.loadClass(appInfo.appComponentFactory).newInstance();
                    delegate.receiver = (AppComponentFactory) factory;
                }
            }

            activeClassLoader = cl;

            // Send real application to attachBaseContext
            attachContext(app, context);

            return app;
        } catch (Exception e) {
            Log.e(DynLoad.class.getSimpleName(), "", e);
            apk.delete();
        }

        activeClassLoader = new StubClassLoader(info);
        return null;
    }

    // Replace LoadedApk mClassLoader
    private static void replaceClassLoader(Context context) {
        // Get ContextImpl
        while (context instanceof ContextWrapper) {
            context = ((ContextWrapper) context).getBaseContext();
        }

        try {
            Field mInfo = context.getClass().getDeclaredField("mPackageInfo");
            mInfo.setAccessible(true);
            Object loadedApk = mInfo.get(context);
            assert loadedApk != null;
            Field mcl = loadedApk.getClass().getDeclaredField("mClassLoader");
            mcl.setAccessible(true);
            mcl.set(loadedApk, new DelegateClassLoader());
        } catch (Exception e) {
            // Actually impossible as this method is only called on API < 29,
            // and API 23 - 28 do not restrict access to these fields.
            Log.e(DynLoad.class.getSimpleName(), "", e);
        }
    }
}
