package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.preference.PreferenceManager;
import android.text.TextUtils;

import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.database.MagiskDatabaseHelper;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.superuser.ContainerApp;
import com.topjohnwu.superuser.Shell;

import java.lang.ref.WeakReference;
import java.util.Locale;
import java.util.Map;

public class MagiskManager extends ContainerApp {

    // Topics
    public final Topic magiskHideDone = new Topic();
    public final Topic reloadActivity = new Topic();
    public final Topic moduleLoadDone = new Topic();
    public final Topic repoLoadDone = new Topic();
    public final Topic updateCheckDone = new Topic();
    public final Topic safetyNetDone = new Topic();
    public final Topic localeDone = new Topic();

    // Info
    public boolean hasInit = false;

    // Data
    public Map<String, Module> moduleMap;

    // Global resources
    public SharedPreferences prefs;
    public MagiskDatabaseHelper mDB;
    public RepoDatabaseHelper repoDB;

    public MagiskManager() {
        Global.weakApp = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER);
        Shell.Config.verboseLogging(BuildConfig.DEBUG);
        Shell.Config.setInitializer(RootUtils.class);

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = MagiskDatabaseHelper.getInstance(this);
        LocaleManager.locale = LocaleManager.defaultLocale = Locale.getDefault();

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

        LocaleManager.setLocale();
        Global.loadConfig();
    }

}
