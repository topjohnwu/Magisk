package com.topjohnwu.magisk;

import android.content.Context;
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

import java.util.List;

public class Global {

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";

    public static class Info {
        public static double magiskVersion;
        public static String magiskVersionString = "(none)";
        public static double remoteMagiskVersion = -1;
        public static String magiskLink;
        public static String releaseNoteLink;
        public static int SNCheckResult = -1;
        public static String bootBlock = null;
        public static boolean isSuClient = false;
        public static String suVersion = null;
        public static boolean disabled = false;
    }
    public static class Data {
        public static ValueSortedMap<String, Repo> repoMap;
        public static ValueSortedMap<String, Module> moduleMap;
        public static List<String> blockList;
        public static List<ApplicationInfo> appList;
        public static List<String> magiskHideList;
        public static void clear() {
            repoMap = null;
            moduleMap = null;
            blockList = null;
            appList = null;
            magiskHideList = null;
        }
    }
    public static class Events {
        public static final CallbackHandler.Event blockDetectionDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event packageLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event reloadMainActivity = new CallbackHandler.Event();
        public static final CallbackHandler.Event moduleLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event repoLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event updateCheckDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event safetyNetDone = new CallbackHandler.Event();
        public static SparseArray<CallbackHandler.Event> uidMap = new SparseArray<>();
    }
    public static class Configs {
        public static boolean isDarkTheme;
        public static boolean shellLogging;
        public static boolean devLogging;
        public static boolean magiskHide;
        public static int suRequestTimeout;
        public static int suLogTimeout = 14;
        public static int suAccessState;
        public static int suResponseType;
        public static int suNotificationType;
    }

    public static void initSuConfigs(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        Configs.suRequestTimeout = Utils.getPrefsInt(prefs, "su_request_timeout", 10);
        Configs.suResponseType = Utils.getPrefsInt(prefs, "su_auto_response", 0);
        Configs.suNotificationType = Utils.getPrefsInt(prefs, "su_notification", 1);
    }

    public static void initSuAccess() {
        List<String> ret = Shell.sh("su -v");
        if (Utils.isValidShellResponse(ret)) {
            Info.suVersion = ret.get(0);
            Info.isSuClient = Info.suVersion.toUpperCase().contains("MAGISK");
        }
        if (Info.isSuClient) {
            ret = Shell.sh("getprop persist.sys.root_access");
            if (Utils.isValidShellResponse(ret))
                Configs.suAccessState = Integer.parseInt(ret.get(0));
            else {
                Shell.su("setprop persist.sys.root_access 3");
                Configs.suAccessState = 3;
            }
        }
    }

    public static void updateMagiskInfo() {
        List<String> ret = Shell.sh("getprop magisk.version");
        if (!Utils.isValidShellResponse(ret)) {
            Info.magiskVersion = -1;
        } else {
            try {
                Info.magiskVersionString = ret.get(0);
                Info.magiskVersion = Double.parseDouble(ret.get(0));
            } catch (NumberFormatException e) {
                // Custom version don't need to receive updates
                Info.magiskVersion = Double.POSITIVE_INFINITY;
            }
        }
        ret = Shell.sh("getprop ro.magisk.disable");
        if (Utils.isValidShellResponse(ret))
            Info.disabled = Integer.parseInt(ret.get(0)) != 0;
    }

}
