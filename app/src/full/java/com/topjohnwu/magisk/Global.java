package com.topjohnwu.magisk;

import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.superuser.ShellUtils;

import java.lang.ref.WeakReference;

public class Global {
    // Global app instance
    public static WeakReference<MagiskManager> weakApp;
    public static Handler mainHandler = new Handler(Looper.getMainLooper());

    // Current status
    public static String magiskVersionString;
    public static int magiskVersionCode = -1;
    public static boolean magiskHide;

    // Update Info
    public static String remoteMagiskVersionString;
    public static int remoteMagiskVersionCode = -1;
    public static String magiskLink;
    public static String magiskNoteLink;
    public static String remoteManagerVersionString;
    public static int remoteManagerVersionCode = -1;
    public static String managerLink;
    public static String managerNoteLink;
    public static String uninstallerLink;

    // Install flags
    public static boolean keepVerity = false;
    public static boolean keepEnc = false;

    public static void loadMagiskInfo() {
        try {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":")[0];
            magiskVersionCode = Integer.parseInt(ShellUtils.fastCmd("magisk -V"));
            String s = ShellUtils.fastCmd((magiskVersionCode >= Const.MAGISK_VER.RESETPROP_PERSIST ?
                    "resetprop -p " : "getprop ") + Const.MAGISKHIDE_PROP);
            magiskHide = s.isEmpty() || Integer.parseInt(s) != 0;
        } catch (NumberFormatException ignored) {}
    }

    public static MagiskManager MM() {
        return weakApp.get();
    }

    public static void toast(CharSequence msg, int duration) {
        mainHandler.post(() -> Toast.makeText(MM(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        mainHandler.post(() -> Toast.makeText(MM(), resId, duration).show());
    }
}
