package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Configuration;

import java.lang.reflect.Method;

public class DelegateApplication extends Application {

    static boolean dynLoad = false;

    private Application receiver;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);

        // Only dynamic load full APK if hidden
        dynLoad = !base.getPackageName().equals(BuildConfig.APPLICATION_ID);

        receiver = InjectAPK.setup(this);
        if (receiver != null) try {
            // Call attachBaseContext without ContextImpl to show it is being wrapped
            Method m = ContextWrapper.class.getDeclaredMethod("attachBaseContext", Context.class);
            m.setAccessible(true);
            m.invoke(receiver, this);
        } catch (Exception ignored) { /* Impossible */ }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (receiver != null)
            receiver.onConfigurationChanged(newConfig);
    }
}
