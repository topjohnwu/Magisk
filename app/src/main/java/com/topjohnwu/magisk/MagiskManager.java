package com.topjohnwu.magisk;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.SafetyNetHelper;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ValueSortedMap;

import java.io.File;
import java.util.List;

public class MagiskManager extends Application {

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_PATH = "/magisk";
    public static final String UNINSTALLER = "magisk_uninstaller.sh";
    public static final String INTENT_SECTION = "section";
    public static final String BUSYBOX_VERSION = "1.26.2";
    public static final String ROOT_ACCESS_PROP = "persist.magisk.root";
    public static final String MULTIUSER_MODE_PROP = "persist.magisk.multiuser";
    public static final String MAGISKHIDE_PROP = "persist.magisk.hide";
    public static final String DISABLE_INDICATION_PROP = "ro.magisk.disable";

    // Events
    public final CallbackEvent<Void> blockDetectionDone = new CallbackEvent<>();
    public final CallbackEvent<Void> magiskHideDone = new CallbackEvent<>();
    public final CallbackEvent<Void> reloadMainActivity = new CallbackEvent<>();
    public final CallbackEvent<Void> moduleLoadDone = new CallbackEvent<>();
    public final CallbackEvent<Void> repoLoadDone = new CallbackEvent<>();
    public final CallbackEvent<Void> updateCheckDone = new CallbackEvent<>();
    public final CallbackEvent<Void> safetyNetDone = new CallbackEvent<>();

    // Info
    public String magiskVersionString;
    public int magiskVersionCode = -1;
    public String remoteMagiskVersionString;
    public int remoteMagiskVersionCode = -1;
    public String magiskLink;
    public String releaseNoteLink;
    public SafetyNetHelper.Result SNCheckResult;
    public String bootBlock = null;
    public boolean isSuClient = false;
    public String suVersion = null;
    public boolean disabled;

    // Data
    public ValueSortedMap<String, Repo> repoMap;
    public ValueSortedMap<String, Module> moduleMap;
    public List<String> blockList;
    public List<ApplicationInfo> appList;
    public List<String> magiskHideList;

    // Configurations
    public static boolean shellLogging;
    public static boolean devLogging;

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

    // Global resources
    public SharedPreferences prefs;
    public SuDatabaseHelper suDB;

    private static Handler mHandler = new Handler();

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        suDB = new SuDatabaseHelper(this);
    }

    public void toast(String msg, int duration) {
        mHandler.post(() -> Toast.makeText(this, msg, duration).show());
    }

    public void toast(int resId, int duration) {
        mHandler.post(() -> Toast.makeText(this, resId, duration).show());
    }

    public void init() {
        isDarkTheme = prefs.getBoolean("dark_theme", false);
        if (BuildConfig.DEBUG) {
            devLogging = prefs.getBoolean("developer_logging", false);
            shellLogging = prefs.getBoolean("shell_logging", false);
        } else {
            devLogging = false;
            shellLogging = false;
        }
        magiskHide = prefs.getBoolean("magiskhide", false);
        updateNotification = prefs.getBoolean("notification", true);
        initSU();
        // Always start a new root shell manually, just for safety
        Shell.init();
        updateMagiskInfo();
        // Initialize busybox
        File busybox = new File(getApplicationInfo().dataDir + "/busybox/busybox");
        if (!busybox.exists() || !TextUtils.equals(prefs.getString("busybox_version", ""), BUSYBOX_VERSION)) {
            busybox.getParentFile().mkdirs();
            Shell.su(
                    "cp -f " + new File(getApplicationInfo().nativeLibraryDir, "libbusybox.so") + " " + busybox,
                    "chmod -R 755 " + busybox.getParent(),
                    busybox + " --install -s " + busybox.getParent()
            );
        }
        // Initialize prefs
        prefs.edit()
                .putBoolean("dark_theme", isDarkTheme)
                .putBoolean("magiskhide", magiskHide)
                .putBoolean("notification", updateNotification)
                .putBoolean("hosts", new File("/magisk/.core/hosts").exists())
                .putBoolean("disable", Utils.itemExist(MAGISK_DISABLE_FILE))
                .putBoolean("su_reauth", suReauth)
                .putString("su_request_timeout", String.valueOf(suRequestTimeout))
                .putString("su_auto_response", String.valueOf(suResponseType))
                .putString("su_notification", String.valueOf(suNotificationType))
                .putString("su_access", String.valueOf(suAccessState))
                .putString("multiuser_mode", String.valueOf(multiuserMode))
                .putString("busybox_version", BUSYBOX_VERSION)
                .apply();
        // Add busybox to PATH
        Shell.su("PATH=$PATH:" + busybox.getParent());
    }

    public void initSUConfig() {
        suRequestTimeout = Utils.getPrefsInt(prefs, "su_request_timeout", 10);
        suResponseType = Utils.getPrefsInt(prefs, "su_auto_response", 0);
        suNotificationType = Utils.getPrefsInt(prefs, "su_notification", 1);
        suReauth = prefs.getBoolean("su_reauth", false);
    }

    public void initSU() {
        // Create the app data directory, so su binary can work properly
        new File(getApplicationInfo().dataDir).mkdirs();

        initSUConfig();

        List<String> ret = Shell.sh("su -v");
        if (Utils.isValidShellResponse(ret)) {
            suVersion = ret.get(0);
            isSuClient = suVersion.toUpperCase().contains("MAGISK");
        }
        if (isSuClient) {
            suAccessState = suDB.getSettings(SuDatabaseHelper.ROOT_ACCESS, 3);
            multiuserMode = suDB.getSettings(SuDatabaseHelper.MULTIUSER_MODE, 0);
        }
    }

    public void updateMagiskInfo() {
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
        ret = Shell.sh("getprop " + DISABLE_INDICATION_PROP);
        try {
            disabled = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            disabled = false;
        }
        ret = Shell.sh("getprop " + MAGISKHIDE_PROP);
        try {
            magiskHide = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            magiskHide = false;
        }
        
    }

}
