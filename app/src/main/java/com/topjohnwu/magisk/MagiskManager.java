package com.topjohnwu.magisk;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.preference.PreferenceManager;
import android.util.SparseArray;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ValueSortedMap;

import java.io.File;
import java.util.List;

public class MagiskManager extends Application {

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";

    // Events
    public final CallbackHandler.Event blockDetectionDone = new CallbackHandler.Event();
    public final CallbackHandler.Event packageLoadDone = new CallbackHandler.Event();
    public final CallbackHandler.Event reloadMainActivity = new CallbackHandler.Event();
    public final CallbackHandler.Event moduleLoadDone = new CallbackHandler.Event();
    public final CallbackHandler.Event repoLoadDone = new CallbackHandler.Event();
    public final CallbackHandler.Event updateCheckDone = new CallbackHandler.Event();
    public final CallbackHandler.Event safetyNetDone = new CallbackHandler.Event();

    // Info
    public double magiskVersion;
    public String magiskVersionString = "(none)";
    public double remoteMagiskVersion = -1;
    public String magiskLink;
    public String releaseNoteLink;
    public int SNCheckResult = -1;
    public String bootBlock = null;
    public boolean isSuClient = false;
    public String suVersion = null;
    public boolean disabled = false;

    // Data
    public ValueSortedMap<String, Repo> repoMap;
    public ValueSortedMap<String, Module> moduleMap;
    public List<String> blockList;
    public List<ApplicationInfo> appList;
    public List<String> magiskHideList;
    public SparseArray<CallbackHandler.Event> uidMap = new SparseArray<>();

    // Configurations
    public static boolean shellLogging;
    public static boolean devLogging;
    public static boolean magiskHide;

    public boolean isDarkTheme;
    public int suRequestTimeout;
    public int suLogTimeout = 14;
    public int suAccessState;
    public int suResponseType;
    public int suNotificationType;

    public SharedPreferences prefs;

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
    }

    public void init() {
        isDarkTheme = prefs.getBoolean("dark_theme", false);
        devLogging = prefs.getBoolean("developer_logging", false);
        shellLogging = prefs.getBoolean("shell_logging", false);
        magiskHide = prefs.getBoolean("magiskhide", false);
        updateMagiskInfo();
        initSuAccess();
        initSuConfigs();
        // Initialize prefs
        prefs.edit()
                .putBoolean("dark_theme", isDarkTheme)
                .putBoolean("magiskhide", magiskHide)
                .putBoolean("busybox", Utils.commandExists("busybox"))
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
            if (Utils.isValidShellResponse(ret))
                suAccessState = Integer.parseInt(ret.get(0));
            else {
                Shell.su(true, "setprop persist.sys.root_access 3");
                suAccessState = 3;
            }
        }
    }

    public void updateMagiskInfo() {
        List<String> ret = Shell.sh("getprop magisk.version");
        if (!Utils.isValidShellResponse(ret)) {
            magiskVersion = -1;
        } else {
            try {
                magiskVersionString = ret.get(0);
                magiskVersion = Double.parseDouble(ret.get(0));
            } catch (NumberFormatException e) {
                // Custom version don't need to receive updates
                magiskVersion = Double.POSITIVE_INFINITY;
            }
        }
        ret = Shell.sh("getprop ro.magisk.disable");
        try {
            disabled = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            disabled = false;
        }
        
    }

}
