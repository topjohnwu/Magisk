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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Field;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class InjectAPK {

    static Object componentFactory;

    private static DelegateComponentFactory getComponentFactory() {
        return (DelegateComponentFactory) componentFactory;
    }

    private static void copy(InputStream src, OutputStream dest) throws IOException {
        try (InputStream s = src) {
            try (OutputStream o = dest) {
                byte[] buf = new byte[8192];
                for (int read; (read = s.read(buf)) >= 0;) {
                    o.write(buf, 0, read);
                }
            }
        }
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    static Application setup(Context context) {
        // Get ContextImpl
        while (context instanceof ContextWrapper) {
            context = ((ContextWrapper) context).getBaseContext();
        }

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
                    copy(new FileInputStream(external), new FileOutputStream(apk));
                } catch (IOException e) {
                    Log.e(InjectAPK.class.getSimpleName(), "", e);
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
                    copy(src, new FileOutputStream(apk));
                }
            } catch (IOException e) {
                Log.e(InjectAPK.class.getSimpleName(), "", e);
                apk.delete();
            }
        }

        if (apk.exists()) {
            ClassLoader cl = new InjectedClassLoader(apk);
            PackageManager pm = context.getPackageManager();
            PackageInfo pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), 0);
            try {
                return createApp(context, cl, pkgInfo.applicationInfo);
            } catch (Exception e) {
                Log.e(InjectAPK.class.getSimpleName(), "", e);
                apk.delete();
            }
            // fallthrough
        }

        ClassLoader cl = new RedirectClassLoader();
        try {
            setClassLoader(context, cl);
            if (Build.VERSION.SDK_INT >= 28) {
                getComponentFactory().loader = cl;
            }
        } catch (Exception e) {
            Log.e(InjectAPK.class.getSimpleName(), "", e);
        }

        return null;
    }

    private static Application createApp(Context context, ClassLoader cl, ApplicationInfo info)
            throws ReflectiveOperationException {
        // Create the receiver Application
        Object app = cl.loadClass(info.className)
                .getConstructor(Object.class)
                .newInstance(DynAPK.pack(dynData()));

        // Create the receiver component factory
        Object factory = null;
        if (Build.VERSION.SDK_INT >= 28) {
            factory = cl.loadClass(info.appComponentFactory).newInstance();
        }

        setClassLoader(context, cl);

        // Finally, set variables
        if (Build.VERSION.SDK_INT >= 28) {
            getComponentFactory().loader = cl;
            getComponentFactory().receiver = (AppComponentFactory) factory;
        }

        return (Application) app;
    }

    // Replace LoadedApk mClassLoader
    private static void setClassLoader(Context impl, ClassLoader cl)
            throws NoSuchFieldException, IllegalAccessException {
        Field mInfo = impl.getClass().getDeclaredField("mPackageInfo");
        mInfo.setAccessible(true);
        Object loadedApk = mInfo.get(impl);
        Field mcl = loadedApk.getClass().getDeclaredField("mClassLoader");
        mcl.setAccessible(true);
        mcl.set(loadedApk, cl);
    }

    private static DynAPK.Data dynData() {
        DynAPK.Data data = new DynAPK.Data();
        data.version = BuildConfig.STUB_VERSION;
        data.classToComponent = Mapping.inverseMap;
        return data;
    }
}
