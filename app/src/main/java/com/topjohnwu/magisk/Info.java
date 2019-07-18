package com.topjohnwu.magisk;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.model.entity.UpdateInfo;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

public final class Info {

    public static int magiskVersionCode = -1;

    @NonNull
    public static String magiskVersionString = "";

    public static UpdateInfo remote = new UpdateInfo();

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
