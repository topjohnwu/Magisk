package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.AppComponentFactory;
import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Configuration;
import android.os.Build;
import android.util.Log;

import com.topjohnwu.magisk.obfuscate.Mapping;
import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;
import java.lang.reflect.Method;

public class DelegateApplication extends Application {

    static File MANAGER_APK;

    private Object factory;
    private Application delegate;

    public DelegateApplication() {}

    public DelegateApplication(Object o) {
        factory = o;
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        if (Build.VERSION.SDK_INT >= 28) {
            setUpDynAPK();
        } else {
            MANAGER_APK = new File(base.getCacheDir(), "app.apk");
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        delegate.onConfigurationChanged(newConfig);
    }

    @SuppressLint("NewApi")
    private void setUpDynAPK() {
        DelegateComponentFactory factory = (DelegateComponentFactory) this.factory;
        MANAGER_APK = DynAPK.current(this);
        File update = DynAPK.update(this);
        if (update.exists())
            update.renameTo(MANAGER_APK);
        if (MANAGER_APK.exists()) {
            ClassLoader cl = new DynamicClassLoader(MANAGER_APK, factory.loader);
            try {
                // Create the delegate AppComponentFactory
                AppComponentFactory df = (AppComponentFactory) cl.loadClass("a.a").newInstance();

                // Create the delegate Application
                delegate = (Application) cl.loadClass("a.e").getConstructor(Object.class)
                        .newInstance(DynAPK.pack(Mapping.data()));

                // Call attachBaseContext without ContextImpl to show it is being wrapped
                Method m = ContextWrapper.class.getDeclaredMethod("attachBaseContext", Context.class);
                m.setAccessible(true);
                m.invoke(delegate, this);

                // If everything went well, set our loader and delegate
                factory.delegate = df;
                factory.loader = cl;
            } catch (Exception e) {
                Log.e(getClass().getSimpleName(), "", e);
                MANAGER_APK.delete();
            }
        }
    }
}
