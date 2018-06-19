package com.topjohnwu.magisk.utils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

public class RootUtils {

    public static void init() {
        Const.MAGISK_DISABLE_FILE = new SuFile("/cache/.disable_magisk");
        SuFile file = new SuFile("/sbin/.core/img");
        if (file.exists()) {
            Const.MAGISK_PATH = file;
        } else if ((file = new SuFile("/dev/magisk/img")).exists()) {
            Const.MAGISK_PATH = file;
        } else {
            Const.MAGISK_PATH = new SuFile("/magisk");
        }
        Const.MAGISK_HOST_FILE = new SuFile(Const.MAGISK_PATH + "/.core/hosts");
    }

    public static void uninstallPkg(String pkg) {
        Shell.Sync.su("db_clean " + Const.USER_ID, "pm uninstall " + pkg);
    }

    public static void patchDTBO() {
        MagiskManager mm = MagiskManager.get();
        if (mm.magiskVersionCode >= Const.MAGISK_VER.DTBO_SUPPORT && !mm.keepVerity) {
            if (ShellUtils.fastCmdResult(Shell.getShell(), "patch_dtbo_image")) {
                ShowUI.dtboPatchedNotification();
            }
        }
    }
}
