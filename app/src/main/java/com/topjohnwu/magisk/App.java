package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDelegate;

import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.database.MagiskDB;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.util.concurrent.ThreadPoolExecutor;

public class App extends Application implements Application.ActivityLifecycleCallbacks {

    public static App self;
    public static Context deContext;
    public static ThreadPoolExecutor THREAD_POOL;

    // Global resources
    public SharedPreferences prefs;
    public MagiskDB mDB;
    public RepoDatabaseHelper repoDB;
    private volatile BaseActivity foreground;

    static {
        AppCompatDelegate.setCompatVectorFromResourcesEnabled(true);
        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER | Shell.FLAG_USE_MAGISK_BUSYBOX);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.addInitializers(RootUtils.class);
        Shell.Config.setTimeout(2);
        THREAD_POOL = (ThreadPoolExecutor) AsyncTask.THREAD_POOL_EXECUTOR;
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        self = this;
        deContext = base;
        registerActivityLifecycleCallbacks(this);

        if (Build.VERSION.SDK_INT >= 24) {
            deContext = base.createDeviceProtectedStorageContext();
            deContext.moveSharedPreferencesFrom(base,
                    PreferenceManager.getDefaultSharedPreferencesName(base));
        }
        prefs = PreferenceManager.getDefaultSharedPreferences(deContext);
        mDB = new MagiskDB(base);

        Networking.init(base);
        LocaleManager.setLocale(this);
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }

    public static BaseActivity foreground() {
        return self.foreground;
    }

    @Override
    public void onActivityCreated(@NonNull Activity activity, @Nullable Bundle bundle) {}

    @Override
    public void onActivityStarted(@NonNull Activity activity) {}

    @Override
    public synchronized void onActivityResumed(@NonNull Activity activity) {
        foreground = (BaseActivity) activity;
    }

    @Override
    public synchronized void onActivityPaused(@NonNull Activity activity) {
        foreground = null;
    }

    @Override
    public void onActivityStopped(@NonNull Activity activity) {}

    @Override
    public void onActivitySaveInstanceState(@NonNull Activity activity, @NonNull Bundle bundle) {}

    @Override
    public void onActivityDestroyed(@NonNull Activity activity) {}
}
