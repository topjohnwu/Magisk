package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Configuration;
import android.os.Build;

import java.lang.reflect.Method;

public class DelegateApplication extends Application {

    private Application delegate;
    static boolean dynLoad = false;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);

        // Only dynamic load full APK if hidden and possible
        dynLoad = Build.VERSION.SDK_INT >= 28 &&
                !base.getPackageName().equals(BuildConfig.APPLICATION_ID);
        if (!dynLoad)
            return;

        delegate = InjectAPK.setup(this);
        if (delegate != null) try {
            // Call attachBaseContext without ContextImpl to show it is being wrapped
            Method m = ContextWrapper.class.getDeclaredMethod("attachBaseContext", Context.class);
            m.setAccessible(true);
            m.invoke(delegate, this);
        } catch (Exception ignored) { /* Impossible */ }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (delegate != null)
            delegate.onConfigurationChanged(newConfig);
    }
}
