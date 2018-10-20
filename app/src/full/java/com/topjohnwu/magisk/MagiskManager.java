package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.preference.PreferenceManager;
import android.text.TextUtils;

import com.topjohnwu.magisk.database.MagiskDatabaseHelper;
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
    public MagiskDatabaseHelper mDB;
    public RepoDatabaseHelper repoDB;

    public MagiskManager() {
        Data.weakApp = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.setInitializer(RootUtils.class);

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = MagiskDatabaseHelper.getInstance();

        String pkg = mDB.getStrings(Const.Key.SU_MANAGER, null);
        if (pkg != null && getPackageName().equals(Const.ORIG_PKG_NAME)) {
            mDB.setStrings(Const.Key.SU_MANAGER, null);
            Shell.su("pm uninstall " + pkg).exec();
        }
        if (TextUtils.equals(pkg, getPackageName())) {
            try {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                getPackageManager().getApplicationInfo(Const.ORIG_PKG_NAME, 0);
                RootUtils.uninstallPkg(Const.ORIG_PKG_NAME);
            } catch (PackageManager.NameNotFoundException ignored) {}
        }

        LocaleManager.setLocale(this);
        Data.loadConfig();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LocaleManager.setLocale(this);
    }
}
