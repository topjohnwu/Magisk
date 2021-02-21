package com.topjohnwu.magisk.utils;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Build;

import com.topjohnwu.magisk.FileProvider;

import java.io.File;

public class APKInstall {

    public static Intent installIntent(Context c, File apk) {
        Intent intent = new Intent(Intent.ACTION_INSTALL_PACKAGE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (Build.VERSION.SDK_INT >= 24) {
            intent.setData(FileProvider.getUriForFile(c, c.getPackageName() + ".provider", apk));
        } else {
            //noinspection ResultOfMethodCallIgnored SetWorldReadable
            apk.setReadable(true, false);
            intent.setData(Uri.fromFile(apk));
        }
        return intent;
    }

    public static void install(Context c, File apk) {
        c.startActivity(installIntent(c, apk));
    }

    public static void registerInstallReceiver(Context c, BroadcastReceiver r) {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_PACKAGE_REPLACED);
        filter.addAction(Intent.ACTION_PACKAGE_ADDED);
        filter.addDataScheme("package");
        c.getApplicationContext().registerReceiver(r, filter);
    }

    public static void installHideResult(Activity c, File apk) {
        Intent intent = installIntent(c, apk);
        intent.putExtra(Intent.EXTRA_RETURN_RESULT, true);
        c.startActivityForResult(intent, 0); // Ignore result, use install receiver
    }
}
