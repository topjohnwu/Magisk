package com.topjohnwu.magisk;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class MagiskManager extends Application {

    // Global weak reference to self
    private static WeakReference<MagiskManager> weakSelf;

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
    public int userId;
    public String magiskVersionString;
    public int magiskVersionCode = -1;
    public String remoteMagiskVersionString;
    public int remoteMagiskVersionCode = -1;
    public String magiskLink;
    public String releaseNoteLink;
    public String remoteManagerVersionString;
    public int remoteManagerVersionCode = -1;
    public String managerLink;
    public String bootBlock = null;
    public int snet_version;
    public int updateServiceVersion;

    // Data
    public Map<String, Module> moduleMap;
    public List<Locale> locales;

    // Configurations
    public static Locale locale;
    public static Locale defaultLocale;

    public boolean magiskHide;
    public boolean isDarkTheme;
    public boolean updateNotification;
    public boolean suReauth;
    public boolean coreOnly;
    public int suRequestTimeout;
    public int suLogTimeout = 14;
    public int suAccessState;
    public int multiuserMode;
    public int suResponseType;
    public int suNotificationType;
    public int suNamespaceMode;
    public String localeConfig;
    public int updateChannel;
    public String bootFormat;
    public String customChannelUrl;

    // Global resources
    public SharedPreferences prefs;
    public SuDatabaseHelper suDB;
    public RepoDatabaseHelper repoDB;
    public Shell shell;
    public Runnable permissionGrantCallback = null;

    private static Handler mHandler = new Handler();

    public MagiskManager() {
        weakSelf = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        userId = getApplicationInfo().uid / 100000;

        if (Utils.getDatabasePath(this, SuDatabaseHelper.DB_NAME).exists()) {
            // Don't migrate yet, wait and check Magisk version
            suDB = new SuDatabaseHelper(this);
        } else {
            suDB = new SuDatabaseHelper();
        }

        // If detect original package, self destruct!
        if (!getPackageName().equals(Const.ORIG_PKG_NAME)) {
            try {
                getPackageManager().getApplicationInfo(Const.ORIG_PKG_NAME, 0);
                Shell.su(String.format(Locale.US, "pm uninstall --user %d %s", userId, getPackageName()));
                return;
            } catch (PackageManager.NameNotFoundException ignored) { /* Expected*/ }
        }

        repoDB = new RepoDatabaseHelper(this);
        defaultLocale = Locale.getDefault();
        setLocale();
        loadConfig();
    }

    public static MagiskManager get() {
        return weakSelf.get();
    }

    public void setLocale() {
        localeConfig = prefs.getString(Const.Key.LOCALE, "");
        if (localeConfig.isEmpty()) {
            locale = defaultLocale;
        } else {
            locale = Locale.forLanguageTag(localeConfig);
        }
        Resources res = getBaseContext().getResources();
        Configuration config = new Configuration(res.getConfiguration());
        config.setLocale(locale);
        res.updateConfiguration(config, res.getDisplayMetrics());
    }

    public void loadConfig() {
        isDarkTheme = prefs.getBoolean(Const.Key.DARK_THEME, false);

        // su
        suRequestTimeout = Utils.getPrefsInt(prefs, Const.Key.SU_REQUEST_TIMEOUT, Const.Value.timeoutList[2]);
        suResponseType = Utils.getPrefsInt(prefs, Const.Key.SU_AUTO_RESPONSE, Const.Value.SU_PROMPT);
        suNotificationType = Utils.getPrefsInt(prefs, Const.Key.SU_NOTIFICATION, Const.Value.NOTIFICATION_TOAST);
        suReauth = prefs.getBoolean(Const.Key.SU_REAUTH, false);
        suAccessState = suDB.getSettings(Const.Key.ROOT_ACCESS, Const.Value.ROOT_ACCESS_APPS_AND_ADB);
        multiuserMode = suDB.getSettings(Const.Key.SU_MULTIUSER_MODE, Const.Value.MULTIUSER_MODE_OWNER_ONLY);
        suNamespaceMode = suDB.getSettings(Const.Key.SU_MNT_NS, Const.Value.NAMESPACE_MODE_REQUESTER);

        coreOnly = prefs.getBoolean(Const.Key.DISABLE, false);
        updateNotification = prefs.getBoolean(Const.Key.UPDATE_NOTIFICATION, true);
        updateChannel = Utils.getPrefsInt(prefs, Const.Key.UPDATE_CHANNEL, Const.Value.STABLE_CHANNEL);
        bootFormat = prefs.getString(Const.Key.BOOT_FORMAT, ".img");
        snet_version = prefs.getInt(Const.Key.SNET_VER, -1);
        updateServiceVersion = prefs.getInt(Const.Key.UPDATE_SERVICE_VER, -1);
        customChannelUrl = prefs.getString(Const.Key.CUSTOM_CHANNEL, "");
    }

    public static void toast(String msg, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), resId, duration).show());
    }

    public void loadMagiskInfo() {
        List<String> ret;
        ret = Shell.sh("magisk -v");
        if (!Utils.isValidShellResponse(ret)) {
            ret = Shell.sh("getprop magisk.version");
            if (Utils.isValidShellResponse(ret)) {
                try {
                    magiskVersionString = ret.get(0);
                    magiskVersionCode = (int) Double.parseDouble(ret.get(0)) * 10;
                } catch (NumberFormatException ignored) {}
            }
        } else {
            magiskVersionString = ret.get(0).split(":")[0];
            ret = Shell.sh("magisk -V");
            try {
                magiskVersionCode = Integer.parseInt(ret.get(0));
            } catch (NumberFormatException ignored) {}
        }
        if (magiskVersionCode > 1435) {
            ret = Shell.su("resetprop -p " + Const.MAGISKHIDE_PROP);
        } else {
            ret = Shell.sh("getprop " + Const.MAGISKHIDE_PROP);
        }
        try {
            magiskHide = !Utils.isValidShellResponse(ret) || Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            magiskHide = true;
        }
    }

    public void setPermissionGrantCallback(Runnable callback) {
        permissionGrantCallback = callback;
    }
}
