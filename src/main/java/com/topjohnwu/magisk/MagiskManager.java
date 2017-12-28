package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Intent;
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
    public boolean keepVerity = false;
    public boolean keepEnc = false;

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
    public int repoOrder;

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

        // Handle duplicate package
        if (!getPackageName().equals(Const.ORIG_PKG_NAME)) {
            try {
                getPackageManager().getApplicationInfo(Const.ORIG_PKG_NAME, 0);
                Intent intent = getPackageManager().getLaunchIntentForPackage(Const.ORIG_PKG_NAME);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                return;
            } catch (PackageManager.NameNotFoundException ignored) { /* Expected */ }
        }

        suDB = SuDatabaseHelper.getSuDB(false);
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
        // su
        suRequestTimeout = Utils.getPrefsInt(prefs, Const.Key.SU_REQUEST_TIMEOUT, Const.Value.timeoutList[2]);
        suResponseType = Utils.getPrefsInt(prefs, Const.Key.SU_AUTO_RESPONSE, Const.Value.SU_PROMPT);
        suNotificationType = Utils.getPrefsInt(prefs, Const.Key.SU_NOTIFICATION, Const.Value.NOTIFICATION_TOAST);
        suReauth = prefs.getBoolean(Const.Key.SU_REAUTH, false);
        suAccessState = suDB.getSettings(Const.Key.ROOT_ACCESS, Const.Value.ROOT_ACCESS_APPS_AND_ADB);
        multiuserMode = suDB.getSettings(Const.Key.SU_MULTIUSER_MODE, Const.Value.MULTIUSER_MODE_OWNER_ONLY);
        suNamespaceMode = suDB.getSettings(Const.Key.SU_MNT_NS, Const.Value.NAMESPACE_MODE_REQUESTER);

        // config
        isDarkTheme = prefs.getBoolean(Const.Key.DARK_THEME, false);
        updateNotification = prefs.getBoolean(Const.Key.UPDATE_NOTIFICATION, true);
        updateChannel = Utils.getPrefsInt(prefs, Const.Key.UPDATE_CHANNEL, Const.Value.STABLE_CHANNEL);
        bootFormat = prefs.getString(Const.Key.BOOT_FORMAT, ".img");
        customChannelUrl = prefs.getString(Const.Key.CUSTOM_CHANNEL, "");
        repoOrder = prefs.getInt(Const.Key.REPO_ORDER, Const.Value.ORDER_NAME);
    }

    public void writeConfig() {
        prefs.edit()
                .putBoolean(Const.Key.DARK_THEME, isDarkTheme)
                .putBoolean(Const.Key.MAGISKHIDE, magiskHide)
                .putBoolean(Const.Key.UPDATE_NOTIFICATION, updateNotification)
                .putBoolean(Const.Key.HOSTS, Utils.itemExist(Const.MAGISK_HOST_FILE()))
                .putBoolean(Const.Key.COREONLY, Utils.itemExist(Const.MAGISK_DISABLE_FILE))
                .putBoolean(Const.Key.SU_REAUTH, suReauth)
                .putString(Const.Key.SU_REQUEST_TIMEOUT, String.valueOf(suRequestTimeout))
                .putString(Const.Key.SU_AUTO_RESPONSE, String.valueOf(suResponseType))
                .putString(Const.Key.SU_NOTIFICATION, String.valueOf(suNotificationType))
                .putString(Const.Key.ROOT_ACCESS, String.valueOf(suAccessState))
                .putString(Const.Key.SU_MULTIUSER_MODE, String.valueOf(multiuserMode))
                .putString(Const.Key.SU_MNT_NS, String.valueOf(suNamespaceMode))
                .putString(Const.Key.UPDATE_CHANNEL, String.valueOf(updateChannel))
                .putString(Const.Key.LOCALE, localeConfig)
                .putString(Const.Key.BOOT_FORMAT, bootFormat)
                .putInt(Const.Key.UPDATE_SERVICE_VER, Const.UPDATE_SERVICE_VER)
                .putInt(Const.Key.REPO_ORDER, repoOrder)
                .apply();
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

        ret = Shell.su("echo \"$BOOTIMAGE\"");
        if (Utils.isValidShellResponse(ret))
            bootBlock = ret.get(0);

        if (suDB != null && !SuDatabaseHelper.verified) {
            suDB.close();
            suDB = SuDatabaseHelper.getSuDB(true);
        }
    }

    public void getDefaultInstallFlags() {
        List<String> ret;
        ret = Shell.su("echo \"$DTBOIMAGE\"");
        if (Utils.isValidShellResponse(ret))
            keepVerity = true;

        ret = Shell.su(
                "getvar KEEPVERITY",
                "echo $KEEPVERITY");
        try {
            if (Utils.isValidShellResponse(ret))
                keepVerity = Boolean.parseBoolean(ret.get(0));
        } catch (NumberFormatException ignored) {}

        ret = Shell.sh("getprop ro.crypto.state");
        if (Utils.isValidShellResponse(ret) && ret.get(0).equals("encrypted"))
            keepEnc = true;

        ret = Shell.su(
                "getvar KEEPFORCEENCRYPT",
                "echo $KEEPFORCEENCRYPT");
        try {
            if (Utils.isValidShellResponse(ret))
                keepEnc = Boolean.parseBoolean(ret.get(0));
        } catch (NumberFormatException ignored) {}
    }

    public void setPermissionGrantCallback(Runnable callback) {
        permissionGrantCallback = callback;
    }
}
