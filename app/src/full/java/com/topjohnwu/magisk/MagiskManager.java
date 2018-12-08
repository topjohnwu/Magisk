package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.preference.PreferenceManager;

import com.topjohnwu.magisk.database.MagiskDB;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.superuser.ContainerApp;
import com.topjohnwu.superuser.Shell;

import java.lang.ref.WeakReference;

public class MagiskManager extends ContainerApp {

    // Info
    public boolean hasInit = false;

    // Global resources
    public SharedPreferences prefs;
    public RepoDatabaseHelper repoDB;
    public MagiskDB mDB;

    public MagiskManager() {
        Data.weakApp = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.setInitializer(RootUtils.class);
        Shell.Config.setTimeout(2);

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        repoDB = new RepoDatabaseHelper(this);
        mDB = new MagiskDB(this);

        LocaleManager.setLocale(this);
        Data.loadConfig();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }
}
