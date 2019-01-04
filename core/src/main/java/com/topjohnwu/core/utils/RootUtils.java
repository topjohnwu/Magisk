package com.topjohnwu.core.utils;

import android.content.Context;

import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.R;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.File;
import java.io.InputStream;

import androidx.annotation.NonNull;

public class RootUtils extends Shell.Initializer {

    public static void rmAndLaunch(String rm, String launch) {
        Shell.su(Utils.fmt("(rm_launch %d %s %s)&", Const.USER_ID, rm, launch)).exec();
    }

    @Override
    public boolean onInit(Context context, @NonNull Shell shell) {
        Shell.Job job = shell.newJob();
        if (shell.isRoot()) {
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
        Data.recovery = Boolean.parseBoolean(ShellUtils.fastCmd("echo $RECOVERYMODE"));
        return true;
    }
}
