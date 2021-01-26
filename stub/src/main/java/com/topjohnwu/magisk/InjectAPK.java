package com.topjohnwu.magisk;

import android.app.AppComponentFactory;
import android.app.Application;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContextWrapper;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Field;

public class InjectAPK {

    @SuppressWarnings("ResultOfMethodCallIgnored")
    static Application setup(Context context) {
        // Get ContextImpl
        while (context instanceof ContextWrapper) {
            context = ((ContextWrapper) context).getBaseContext();
        }

        File apk = DynAPK.current(context);
        File update = DynAPK.update(context);
        if (update.exists())
            update.renameTo(apk);
        Application result = null;
        if (!apk.exists()) {
            // Try copying APK
            Uri uri = new Uri.Builder().scheme("content")
                    .authority("com.topjohnwu.magisk.provider")
                    .encodedPath("apk_file").build();
            ContentResolver resolver = context.getContentResolver();
            try (InputStream src = resolver.openInputStream(uri)) {
                if (src != null) {
                    try (OutputStream out = new FileOutputStream(apk)) {
                        byte[] buf = new byte[4096];
                        for (int read; (read = src.read(buf)) >= 0;) {
                            out.write(buf, 0, read);
                        }
                    }
                }
            } catch (Exception ignored) {}
        }
        if (apk.exists()) {
            ClassLoader cl = new InjectedClassLoader(apk);
            try {
                // Create the receiver Application
                Object app = cl.loadClass(Mapping.get("APP")).getConstructor(Object.class)
                        .newInstance(DynAPK.pack(dynData()));

                // Create the receiver component factory
                Object factory = null;
                if (Build.VERSION.SDK_INT >= 28) {
                    factory = cl.loadClass(Mapping.get("ACF")).newInstance();
                }

                setClassLoader(context, cl);

                // Finally, set variables
                result = (Application) app;
                if (Build.VERSION.SDK_INT >= 28) {
                    DelegateComponentFactory.INSTANCE.loader = cl;
                    DelegateComponentFactory.INSTANCE.receiver = (AppComponentFactory) factory;
                }
            } catch (Exception e) {
                Log.e(InjectAPK.class.getSimpleName(), "", e);
                apk.delete();
            }
        } else {
            ClassLoader cl = new RedirectClassLoader();
            try {
                setClassLoader(context, cl);
                if (Build.VERSION.SDK_INT >= 28) {
                    DelegateComponentFactory.INSTANCE.loader = cl;
                }
            } catch (Exception e) {
                // Should never happen
                Log.e(InjectAPK.class.getSimpleName(), "", e);
            }
        }
        return result;
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
