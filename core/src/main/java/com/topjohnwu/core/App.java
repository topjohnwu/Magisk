package com.topjohnwu.core;

import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Handler;
import android.os.Looper;
import android.preference.PreferenceManager;

import com.topjohnwu.core.database.MagiskDB;
import com.topjohnwu.core.database.RepoDatabaseHelper;
import com.topjohnwu.core.utils.LocaleManager;
import com.topjohnwu.core.utils.RootUtils;
import com.topjohnwu.superuser.ContainerApp;
import com.topjohnwu.superuser.Shell;

public class App extends ContainerApp {

    public static App self;
    public static Handler mainHandler = new Handler(Looper.getMainLooper());
    public boolean init = false;

    // Global resources
    public SharedPreferences prefs;
    public MagiskDB mDB;
    public RepoDatabaseHelper repoDB;

    public App() {
        self = this;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.setInitializer(RootUtils.class);
        Shell.Config.setTimeout(2);

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = new MagiskDB(this);
        repoDB = new RepoDatabaseHelper(this);

        LocaleManager.setLocale(this);
        Data.loadConfig();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }
}
