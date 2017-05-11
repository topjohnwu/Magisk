package com.topjohnwu.magisk;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.SparseArray;
import android.widget.Toast;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.superuser.Policy;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ValueSortedMap;

import java.io.File;
import java.util.List;

public class MagiskManager extends Application {

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";
    public static final String MAGISK_HIDE_PATH = "/magisk/.core/magiskhide/";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_PATH = "/magisk";
    public static final String INTENT_SECTION = "section";

    // Events
    public final CallbackEvent<Void> blockDetectionDone = new CallbackEvent<>();
    public final CallbackEvent<Void> magiskHideDone = new CallbackEvent<>();
    public final CallbackEvent<Void> reloadMainActivity = new CallbackEvent<>();
    public final CallbackEvent<Void> moduleLoadDone = new CallbackEvent<>();
    public final CallbackEvent<Void> repoLoadDone = new CallbackEvent<>();
    public final CallbackEvent<Void> updateCheckDone = new CallbackEvent<>();
    public final CallbackEvent<Void> safetyNetDone = new CallbackEvent<>();
    public final SparseArray<CallbackEvent<Policy>> uidSuRequest = new SparseArray<>();

    // Info
    public String magiskVersionString;
    public int magiskVersionCode = -1;
    public String remoteMagiskVersionString;
    public int remoteMagiskVersionCode = -1;
    public String magiskLink;
    public String releaseNoteLink;
    public int SNCheckResult = -1;
    public String bootBlock = null;
    public boolean isSuClient = false;
    public String suVersion = null;
    public boolean disabled;
    public boolean magiskHideStarted;

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
    public boolean busybox;
    public int suRequestTimeout;
    public int suLogTimeout = 14;
    public int suAccessState;
    public int suResponseType;
    public int suNotificationType;

    public SharedPreferences prefs;

    private static Handler mHandler = new Handler();

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
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
        // Always start a new root shell manually, just for safety
        Shell.init();
        updateMagiskInfo();
        initSuAccess();
        initSuConfigs();
        // Initialize prefs
        prefs.edit()
                .putBoolean("dark_theme", isDarkTheme)
                .putBoolean("magiskhide", magiskHide)
                .putBoolean("notification", updateNotification)
                .putBoolean("busybox", busybox)
                .putBoolean("hosts", new File("/magisk/.core/hosts").exists())
                .putBoolean("disable", Utils.itemExist(MAGISK_DISABLE_FILE))
                .putString("su_request_timeout", String.valueOf(suRequestTimeout))
                .putString("su_auto_response", String.valueOf(suResponseType))
                .putString("su_notification", String.valueOf(suNotificationType))
                .putString("su_access", String.valueOf(suAccessState))
                .apply();
    }

    public void initSuConfigs() {
        suRequestTimeout = Utils.getPrefsInt(prefs, "su_request_timeout", 10);
        suResponseType = Utils.getPrefsInt(prefs, "su_auto_response", 0);
        suNotificationType = Utils.getPrefsInt(prefs, "su_notification", 1);
    }

    public void initSuAccess() {
        List<String> ret = Shell.sh("su -v");
        if (Utils.isValidShellResponse(ret)) {
            suVersion = ret.get(0);
            isSuClient = suVersion.toUpperCase().contains("MAGISK");
        }
        if (isSuClient) {
            ret = Shell.sh("getprop persist.sys.root_access");
            if (Utils.isValidShellResponse(ret)) {
                suAccessState = Integer.parseInt(ret.get(0));
            } else {
                Shell.su(true, "setprop persist.sys.root_access 3");
                suAccessState = 3;
            }
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
        ret = Shell.sh("getprop persist.magisk.busybox");
        try {
            busybox = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            busybox = false;
        }
        ret = Shell.sh("getprop ro.magisk.disable");
        try {
            disabled = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            disabled = false;
        }
        ret = Shell.sh("getprop persist.magisk.hide");
        try {
            magiskHideStarted = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            magiskHideStarted = false;
        }

        if (magiskHideStarted) {
            magiskHide = true;
        }
        
    }

}
