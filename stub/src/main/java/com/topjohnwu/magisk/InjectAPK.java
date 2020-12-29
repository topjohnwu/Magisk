package com.topjohnwu.magisk;

import android.app.AppComponentFactory;
import android.app.Application;
import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;

public class InjectAPK {

    static DelegateComponentFactory factory;

    static Application setup(Context context) {
        File apk = DynAPK.current(context);
        File update = DynAPK.update(context);
        if (update.exists())
            update.renameTo(apk);
        Application delegate = null;
        if (!apk.exists()) {
            // Try copying APK
            Uri uri = new Uri.Builder().scheme("content")
                    .authority("com.topjohnwu.magisk.provider")
                    .encodedPath("apk_file").build();
            ContentResolver resolver = context.getContentResolver();
            try (InputStream is = resolver.openInputStream(uri)) {
                if (is != null) {
                    try (OutputStream out = new FileOutputStream(apk)) {
                        byte[] buf = new byte[4096];
                        for (int read; (read = is.read(buf)) >= 0;) {
                            out.write(buf, 0, read);
                        }
                    }
                }
            } catch (Exception e) {
                Log.e(InjectAPK.class.getSimpleName(), "", e);
            }
        }
        if (apk.exists()) {
            ClassLoader cl = new DynamicClassLoader(apk, factory.loader);
            try {
                // Create the delegate AppComponentFactory
                AppComponentFactory df = (AppComponentFactory)
                        cl.loadClass("androidx.core.app.CoreComponentFactory").newInstance();

                // Create the delegate Application
                delegate = (Application) cl.loadClass("a.e").getConstructor(Object.class)
                        .newInstance(DynAPK.pack(dynData()));

                // If everything went well, set our loader and delegate
                factory.delegate = df;
                factory.loader = cl;
            } catch (Exception e) {
                Log.e(InjectAPK.class.getSimpleName(), "", e);
                apk.delete();
            }
        }
        return delegate;
    }

    private static DynAPK.Data dynData() {
        DynAPK.Data data = new DynAPK.Data();
        data.version = BuildConfig.STUB_VERSION;
        // Public source code does not do component name obfuscation
        data.classToComponent = new HashMap<>();
        return data;
    }

}
