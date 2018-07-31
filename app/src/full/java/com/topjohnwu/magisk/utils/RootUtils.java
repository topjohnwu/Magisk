package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.support.annotation.NonNull;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.BusyBox;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class RootUtils extends Shell.Initializer {

    static {
        BusyBox.BB_PATH = new File(Const.BUSYBOX_PATH);
    }

    public static void uninstallPkg(String pkg) {
        Shell.su("db_clean " + Const.USER_ID, "pm uninstall " + pkg).exec();
    }

    @Override
    public boolean onInit(Context context, @NonNull Shell shell) {
        if (shell.isRoot()) {
            try (InputStream magiskUtils = context.getResources().openRawResource(R.raw.util_functions);
                 InputStream managerUtils = context.getResources().openRawResource(R.raw.utils)
            ) {
                shell.newJob()
                        .add(magiskUtils).add(managerUtils)
                        .add("mount_partitions", "get_flags", "run_migrations")
                        .exec();
            } catch (IOException e) {
                return false;
            }

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

            Data.loadMagiskInfo();

            Data.keepVerity = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPVERITY"));
            Data.keepEnc = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPFORCEENCRYPT"));
        }
        return true;
    }
}
