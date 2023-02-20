package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Context;
import android.content.res.Configuration;

public class DelegateApplication extends Application {

    private Application receiver;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        receiver = DynLoad.createAndSetupApp(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (receiver != null)
            receiver.onCreate();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (receiver != null)
            receiver.onConfigurationChanged(newConfig);
    }
}
