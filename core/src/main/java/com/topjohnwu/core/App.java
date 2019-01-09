package com.topjohnwu.core;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Handler;
import android.os.Looper;
import android.preference.PreferenceManager;

import com.topjohnwu.core.database.MagiskDB;
import com.topjohnwu.core.database.RepoDatabaseHelper;
import com.topjohnwu.core.utils.LocaleManager;
import com.topjohnwu.core.utils.RootUtils;
import com.topjohnwu.net.Networking;
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

    private Resources mResource;

    static {
        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER | Shell.FLAG_USE_MAGISK_BUSYBOX);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.addInitializers(RootUtils.class);
        Shell.Config.setTimeout(2);
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        self = this;
        mResource = base.getResources();

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = new MagiskDB(this);
        repoDB = new RepoDatabaseHelper(this);

        Networking.init(this);
        LocaleManager.setLocale(this);
        Data.loadConfig();
    }

    @Override
    public Resources getResources() {
        return mResource;
    }

    public void setResources(Resources res) {
        mResource = res;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }
}
