package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.preference.PreferenceManager;

import com.topjohnwu.magisk.core.BuildConfig;
import com.topjohnwu.magisk.database.MagiskDB;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.util.concurrent.ThreadPoolExecutor;

public class App extends Application {

    public static App self;
    public static ThreadPoolExecutor THREAD_POOL;

    // Global resources
    public SharedPreferences prefs;
    public MagiskDB mDB;
    public RepoDatabaseHelper repoDB;

    static {
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

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = new MagiskDB(this);
        repoDB = new RepoDatabaseHelper(this);

        Networking.init(this);
        LocaleManager.setLocale(this);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }
}
