package com.topjohnwu.magisk;

import android.app.AppComponentFactory;
import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import com.topjohnwu.magisk.utils.APKInstall;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
@SuppressWarnings("ResultOfMethodCallIgnored")
public class DynLoad {

    // The current active classloader
    static ClassLoader loader = new RedirectClassLoader();
    static Object componentFactory;
    static final DynAPK.Data apkData = createApkData();

    private static boolean loadedApk = false;

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
    static void loadAPK(ApplicationInfo info) {
        if (loadedApk)
            return;
        loadedApk = true;

        File apk = DynAPK.current(info);
        File update = DynAPK.update(info);

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

    // Dynamically load APK, inject ClassLoader into ContextImpl, then
    // create the non-stub Application instance from the loaded APK
    static Application createApp(Context context) {
        File apk = DynAPK.current(context);
        loadAPK(context.getApplicationInfo());

        // Trigger folder creation
        context.getExternalFilesDir(null);

        // If no APK to load, attempt to copy from previous app
        if (!isDynLoader() && !context.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
            Uri uri = new Uri.Builder().scheme("content")
                    .authority("com.topjohnwu.magisk.provider")
                    .encodedPath("apk_file").build();
            try {
                InputStream src = context.getContentResolver().openInputStream(uri);
                if (src != null) {
                    var out = new FileOutputStream(apk);
                    try (src; out) {
                        APKInstall.transfer(src, out);
                    }
                    loader = new InjectedClassLoader(apk);
                }
            } catch (IOException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }
        }

        if (isDynLoader()) {
            PackageManager pm = context.getPackageManager();
            PackageInfo pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), 0);
            try {
                return newApp(pkgInfo.applicationInfo);
            } catch (ReflectiveOperationException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }

        }
        return null;
    }

    // Stub app setup entry
    static Application createAndSetupApp(Application context) {
        // On API >= 29, AppComponentFactory will replace the ClassLoader
        if (Build.VERSION.SDK_INT < 29)
            replaceClassLoader(context);

        Application app = createApp(context);
        if (app != null) {
            // Send real application to attachBaseContext
            attachContext(app, context);
        }
        return app;
    }

    private static boolean isDynLoader() {
        return loader instanceof InjectedClassLoader;
    }

    private static Application newApp(ApplicationInfo info) throws ReflectiveOperationException {
        // Create the receiver Application
        var app = (Application) loader.loadClass(info.className)
                .getConstructor(Object.class)
                .newInstance(apkData.getObject());

        // Create the receiver component factory
        if (Build.VERSION.SDK_INT >= 28 && componentFactory != null) {
            Object factory = loader.loadClass(info.appComponentFactory).newInstance();
            var delegate = (DelegateComponentFactory) componentFactory;
            delegate.receiver = (AppComponentFactory) factory;
        }

        return app;
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
            // and API 21 - 28 do not restrict access to these methods/fields.
            Log.e(DynLoad.class.getSimpleName(), "", e);
        }
    }

    private static DynAPK.Data createApkData() {
        var data = new DynAPK.Data();
        data.setVersion(BuildConfig.STUB_VERSION);
        data.setClassToComponent(Mapping.inverseMap);
        data.setRootService(DelegateRootService.class);
        return data;
    }
}
