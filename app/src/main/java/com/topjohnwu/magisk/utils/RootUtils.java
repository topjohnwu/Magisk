package com.topjohnwu.magisk.utils;

import android.content.ComponentName;
import android.content.Context;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.InputStream;

public class RootUtils extends Shell.Initializer {

    public static void rmAndLaunch(String rm, ComponentName component) {
        Shell.su(Utils.fmt("(rm_launch %s %s)&", rm, component.flattenToString())).exec();
    }

    @Override
    public boolean onInit(Context context, @NonNull Shell shell) {
        Shell.Job job = shell.newJob();
        if (shell.isRoot()) {
            job.add(context.getResources().openRawResource(R.raw.util_functions))
                .add(context.getResources().openRawResource(R.raw.utils));
            Const.MAGISK_DISABLE_FILE = new SuFile("/cache/.disable_magisk");
            Config.loadMagiskInfo();
        } else {
            InputStream nonroot = context.getResources().openRawResource(R.raw.nonroot_utils);
            job.add(nonroot);
        }

        job.add("mount_partitions", "get_flags", "run_migrations", "export BOOTMODE=true").exec();

        Config.keepVerity = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPVERITY"));
        Config.keepEnc = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPFORCEENCRYPT"));
        Config.recovery = Boolean.parseBoolean(ShellUtils.fastCmd("echo $RECOVERYMODE"));
        return true;
    }
}
