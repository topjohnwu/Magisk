package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;

import com.topjohnwu.magisk.FileProvider;

import java.io.File;

public class APKInstall {
    public static void install(Context c, File apk) {
        c.startActivity(installIntent(c, apk));
    }

    public static Intent installIntent(Context c, File apk) {
        Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
        install.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        install.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            install.setData(FileProvider.getUriForFile(c, c.getPackageName() + ".provider", apk));
        } else {
            apk.setReadable(true, false);
            install.setData(Uri.fromFile(apk));
        }
        return install;
    }
}
