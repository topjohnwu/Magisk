package com.topjohnwu.magisk;

import android.app.AppComponentFactory;
import android.app.Application;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import com.topjohnwu.magisk.utils.APKInstall;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
@SuppressWarnings("ResultOfMethodCallIgnored")
public class DynLoad {

    static Object componentFactory;
    static final DynAPK.Data apkData = createApkData();

    // Dynamically load APK, inject ClassLoader into ContextImpl, then
    // create the actual Application instance from the loaded APK
    static Application inject(Context context) {
        File apk = DynAPK.current(context);
        File update = DynAPK.update(context);

        if (update.exists()) {
            // Rename from update
            update.renameTo(apk);
        }

        if (BuildConfig.DEBUG) {
            // Copy from external for easier development
            File external = new File(context.getExternalFilesDir(null), "magisk.apk");
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

        if (!apk.exists() && !context.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
            // Copy from previous app
            Uri uri = new Uri.Builder().scheme("content")
                    .authority("com.topjohnwu.magisk.provider")
                    .encodedPath("apk_file").build();
            ContentResolver resolver = context.getContentResolver();
            try {
                InputStream src = resolver.openInputStream(uri);
                if (src != null) {
                    var out = new FileOutputStream(apk);
                    try (src; out) {
                        APKInstall.transfer(src, out);
                    }
                }
            } catch (IOException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }
        }

        if (apk.exists()) {
            ClassLoader cl = new InjectedClassLoader(apk);
            PackageManager pm = context.getPackageManager();
            PackageInfo pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), 0);
            try {
                return createApp(context, cl, pkgInfo.applicationInfo);
            } catch (ReflectiveOperationException e) {
                Log.e(DynLoad.class.getSimpleName(), "", e);
                apk.delete();
            }

        }
        return null;
    }

    // Inject and create Application, or setup redirections for the current app
    static Application setup(Context context) {
        Application app = inject(context);
        if (app != null) {
            return app;
        }

        ClassLoader cl = new RedirectClassLoader();
        try {
            setClassLoader(context, cl);
            if (Build.VERSION.SDK_INT >= 28) {
                ((DelegateComponentFactory) componentFactory).loader = cl;
            }
        } catch (Exception e) {
            Log.e(DynLoad.class.getSimpleName(), "", e);
        }

        return null;
    }

    private static Application createApp(Context context, ClassLoader cl, ApplicationInfo info)
            throws ReflectiveOperationException {
        // Create the receiver Application
        Object app = cl.loadClass(info.className)
                .getConstructor(Object.class)
                .newInstance(apkData.getObject());

        // Create the receiver component factory
        if (Build.VERSION.SDK_INT >= 28 && componentFactory != null) {
            Object factory = cl.loadClass(info.appComponentFactory).newInstance();
            DelegateComponentFactory delegate = (DelegateComponentFactory) componentFactory;
            delegate.loader = cl;
            delegate.receiver = (AppComponentFactory) factory;
        }

        setClassLoader(context, cl);

        return (Application) app;
    }

    // Replace LoadedApk mClassLoader
    private static void setClassLoader(Context context, ClassLoader cl)
            throws NoSuchFieldException, IllegalAccessException {
        // Get ContextImpl
        while (context instanceof ContextWrapper) {
            context = ((ContextWrapper) context).getBaseContext();
        }

        Field mInfo = context.getClass().getDeclaredField("mPackageInfo");
        mInfo.setAccessible(true);
        Object loadedApk = mInfo.get(context);
        assert loadedApk != null;
        Field mcl = loadedApk.getClass().getDeclaredField("mClassLoader");
        mcl.setAccessible(true);
        mcl.set(loadedApk, cl);
    }

    private static DynAPK.Data createApkData() {
        DynAPK.Data data = new DynAPK.Data();
        data.setVersion(BuildConfig.STUB_VERSION);
        data.setClassToComponent(Mapping.inverseMap);
        data.setRootService(DelegateRootService.class);
        return data;
    }
}
