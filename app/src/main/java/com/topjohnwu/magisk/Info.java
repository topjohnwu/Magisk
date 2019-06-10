package com.topjohnwu.magisk;

import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

public final class Info {

    public static int magiskVersionCode = -1;
    // Current status
    public static String magiskVersionString = "";
    // Update Info
    public static String remoteMagiskVersionString = "";
    public static int remoteMagiskVersionCode = -1;
    public static String magiskLink = "";
    public static String magiskNoteLink = "";
    public static String magiskMD5 = "";
    public static String remoteManagerVersionString = "";
    public static int remoteManagerVersionCode = -1;
    public static String managerLink = "";
    public static String managerNoteLink = "";
    public static String uninstallerLink = "";

    // Install flags
    public static boolean keepVerity = false;
    public static boolean keepEnc = false;
    public static boolean recovery = false;

    public static void loadMagiskInfo() {
        try {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":")[0];
            magiskVersionCode = Integer.parseInt(ShellUtils.fastCmd("magisk -V"));
            Config.setMagiskHide(Shell.su("magiskhide --status").exec().isSuccess());
        } catch (NumberFormatException ignored) {
        }
    }
}
