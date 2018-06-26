package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.support.annotation.NonNull;

import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.BusyBox;
import com.topjohnwu.superuser.Shell;

import java.io.File;
import java.io.InputStream;

public class ShellInitializer extends Shell.Initializer {

    @Override
    public boolean onRootShellInit(Context context, @NonNull Shell shell) throws Exception {
        BusyBox.BB_PATH = new File(Const.BUSYBOX_PATH);
        try (InputStream magiskUtils = context.getResources().openRawResource(R.raw.util_functions);
             InputStream managerUtils = context.getResources().openRawResource(R.raw.utils)
        ) {
            shell.loadInputStream(null, null, magiskUtils);
            shell.loadInputStream(null, null, managerUtils);
        }
        shell.run(null, null,
                "mount_partitions",
                "get_flags",
                "run_migrations");
        return true;
    }
}
