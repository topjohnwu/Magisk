package com.topjohnwu.magisk.utils;

import android.content.Context;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.BusyBox;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.File;
import java.io.InputStream;

import androidx.annotation.NonNull;

public class RootUtils extends Shell.Initializer {

    static {
        BusyBox.BB_PATH = new File(Const.BUSYBOX_PATH);
    }

    public static void uninstallPkg(String pkg) {
        Shell.su("db_clean " + Const.USER_ID, "pm uninstall " + pkg).exec();
    }

    @Override
    public boolean onInit(Context context, @NonNull Shell shell) {
        Shell.Job job = shell.newJob();
        if (shell.isRoot()) {
            if (!new SuFile("/sbin/.magisk").exists())
                job.add("ln -s /sbin/.core /sbin/.magisk");

            job.add(context.getResources().openRawResource(R.raw.util_functions))
                .add(context.getResources().openRawResource(R.raw.utils));
            Const.MAGISK_DISABLE_FILE = new SuFile("/cache/.disable_magisk");
            Data.loadMagiskInfo();
        } else {
            InputStream nonroot = context.getResources().openRawResource(R.raw.nonroot_utils);
            job.add(nonroot);
        }

        job.add("mount_partitions", "get_flags", "run_migrations", "export BOOTMODE=true").exec();

        Data.keepVerity = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPVERITY"));
        Data.keepEnc = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPFORCEENCRYPT"));
        return true;
    }
}
