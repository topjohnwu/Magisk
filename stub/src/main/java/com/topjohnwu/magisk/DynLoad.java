package com.topjohnwu.magisk;

import static com.topjohnwu.magisk.BuildConfig.APPLICATION_ID;

import android.app.AppComponentFactory;
import android.app.Application;
import android.app.job.JobService;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import com.topjohnwu.magisk.dummy.DummyProvider;
import com.topjohnwu.magisk.dummy.DummyReceiver;
import com.topjohnwu.magisk.dummy.DummyService;
import com.topjohnwu.magisk.utils.APKInstall;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
@SuppressWarnings("ResultOfMethodCallIgnored")
public class DynLoad {

    // The current active classloader
    static ClassLoader loader = DynLoad.class.getClassLoader();
    static Object componentFactory;
    static Map<String, String> componentMap = new HashMap<>();

    private static boolean loadedApk = false;

    static StubApk.Data createApkData() {
        var data = new StubApk.Data();
        data.setVersion(BuildConfig.STUB_VERSION);
        Map<String, String> map = new HashMap<>();
        for (var e : componentMap.entrySet()) {
            map.put(e.getValue(), e.getKey());
        }
        data.setClassToComponent(map);
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

    // Dynamically load APK from internal or external storage
    static void loadApk(ApplicationInfo info) {
        if (loadedApk)
            return;
        loadedApk = true;

        File apk = StubApk.current(info);
        File update = StubApk.update(info);

        if (update.exists()) {
            // Rename from update
            update.renameTo(apk);
        }

        // Copy from external for easier development
        if (BuildConfig.DEBUG) copy_from_ext: {
            final File dir;
            try {
                var dirs = (File[]) Environment.class
                        .getMethod("buildExternalStorageAppFilesDirs", String.class)
                        .invoke(null, info.packageName);
                if (dirs == null)
                    break copy_from_ext;
                dir = dirs[0];
            } catch (ReflectiveOperationException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                break copy_from_ext;
            }
            File external = new File(dir, "magisk.apk");
            if (external.exists()) {
                try {
                    var in = new FileInputStream(external);
                    var out = new FileOutputStream(apk);
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
        }

        if (apk.exists()) {
           loader = new InjectedClassLoader(apk);
        }
    }

    // Dynamically load APK from internal, external storage, or previous app
    static boolean loadApk(Context context) {
        // Trigger folder creation
        context.getExternalFilesDir(null);

        loadApk(context.getApplicationInfo());

        // If no APK is loaded, attempt to copy from previous app
        if (!isDynLoader() && !context.getPackageName().equals(APPLICATION_ID)) {
            File apk = StubApk.current(context);
            try {
                var info = context.getPackageManager().getApplicationInfo(APPLICATION_ID, 0);
                var src = new FileInputStream(info.sourceDir);
                var out = new FileOutputStream(apk);
                try (src; out) {
                    APKInstall.transfer(src, out);
                }
                loader = new InjectedClassLoader(apk);
            } catch (PackageManager.NameNotFoundException ignored) {
            } catch (IOException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }
        }

        return isDynLoader();
    }

    // Dynamically load APK and create the Application instance from the loaded APK
    static Application createAndSetupApp(Application context) {
        // On API >= 29, AppComponentFactory will replace the ClassLoader for us
        if (Build.VERSION.SDK_INT < 29)
            replaceClassLoader(context);

        int flags = PackageManager.GET_ACTIVITIES | PackageManager.GET_SERVICES
                | PackageManager.GET_PROVIDERS | PackageManager.GET_RECEIVERS;

        final PackageInfo info;
        try {
            info = context.getPackageManager()
                    .getPackageInfo(context.getPackageName(), flags);
        } catch (PackageManager.NameNotFoundException e) {
            // Impossible
            throw new RuntimeException(e);
        }

        if (!loadApk(context)) {
            loader = new RedirectClassLoader(createInternalMap(info));
            return null;
        }

        File apk = StubApk.current(context);
        PackageManager pm = context.getPackageManager();
        try {
            var pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), flags);
            var appInfo = pkgInfo.applicationInfo;

            updateComponentMap(info, pkgInfo);

            // Create the receiver Application
            var data = createApkData();
            var app = (Application) loader.loadClass(appInfo.className)
                    .getConstructor(Object.class)
                    .newInstance(data.getObject());

            // Create the receiver component factory
            if (Build.VERSION.SDK_INT >= 28 && componentFactory != null) {
                Object factory = loader.loadClass(appInfo.appComponentFactory).newInstance();
                var delegate = (DelegateComponentFactory) componentFactory;
                delegate.receiver = (AppComponentFactory) factory;
            }

            // Send real application to attachBaseContext
            attachContext(app, context);

            return app;
        } catch (Exception e) {
            Log.e(DynLoad.class.getSimpleName(), "", e);
            apk.delete();
        }

        return null;
    }

    private static Map<String, Class<?>> createInternalMap(PackageInfo info) {
        Map<String, Class<?>> map = new HashMap<>();
        for (var c : info.activities) {
            map.put(c.name, DownloadActivity.class);
        }
        for (var c : info.services) {
            map.put(c.name, DummyService.class);
        }
        for (var c : info.providers) {
            map.put(c.name, DummyProvider.class);
        }
        for (var c : info.receivers) {
            map.put(c.name, DummyReceiver.class);
        }
        return map;
    }

    private static void updateComponentMap(PackageInfo from, PackageInfo to) {
        {
            var src = from.activities;
            var dest = to.activities;

            final ActivityInfo sa;
            final ActivityInfo da;
            final ActivityInfo sb;
            final ActivityInfo db;
            if (src[0].exported) {
                sa = src[0];
                sb = src[1];
            } else {
                sa = src[1];
                sb = src[0];
            }
            if (dest[0].exported) {
                da = dest[0];
                db = dest[1];
            } else {
                da = dest[1];
                db = dest[0];
            }
            componentMap.put(sa.name, da.name);
            componentMap.put(sb.name, db.name);
        }

        {
            var src = from.services;
            var dest = to.services;

            final ServiceInfo sa;
            final ServiceInfo da;
            final ServiceInfo sb;
            final ServiceInfo db;
            if (JobService.PERMISSION_BIND.equals(src[0].permission)) {
                sa = src[0];
                sb = src[1];
            } else {
                sa = src[1];
                sb = src[0];
            }
            if (JobService.PERMISSION_BIND.equals(dest[0].permission)) {
                da = dest[0];
                db = dest[1];
            } else {
                da = dest[1];
                db = dest[0];
            }
            componentMap.put(sa.name, da.name);
            componentMap.put(sb.name, db.name);
        }

        {
            var src = from.receivers;
            var dest = to.receivers;
            componentMap.put(src[0].name, dest[0].name);
        }

        {
            var src = from.providers;
            var dest = to.providers;
            componentMap.put(src[0].name, dest[0].name);
        }
    }

    static boolean isDynLoader() {
        return loader instanceof InjectedClassLoader;
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
            Field mcl = loadedApk.getClass().getDeclaredField("mClassLoader");
            mcl.setAccessible(true);
            mcl.set(loadedApk, new DelegateClassLoader());
        } catch (Exception e) {
            // Actually impossible as this method is only called on API < 29,
            // and API 21 - 28 do not restrict access to these fields.
            Log.e(DynLoad.class.getSimpleName(), "", e);
        }
    }
}
