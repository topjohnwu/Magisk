package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;

import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.PatchAPK;
import com.topjohnwu.magisk.utils.Utils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

public class ManagerUpdate extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Utils.dlAndReceive(
                context, new PatchedInstall(),
                intent.getStringExtra(Const.Key.INTENT_SET_LINK),
                intent.getStringExtra(Const.Key.INTENT_SET_FILENAME)
        );
    }

    private static class PatchedInstall extends ManagerInstall {
        @Override
        public void onDownloadDone(Context context, Uri uri) {
            if (!context.getPackageName().equals(Const.ORIG_PKG_NAME)) {
                AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                    String o = uri.getPath();
                    String p = o.substring(0, o.lastIndexOf('.')) + "-patched.apk";
                    try {
                        PatchAPK.patchPackageID(o, new BufferedOutputStream(new FileOutputStream(p)),
                                Const.ORIG_PKG_NAME, context.getPackageName());
                    } catch (FileNotFoundException ignored) { }
                    super.onDownloadDone(context, Uri.fromFile(new File(p)));
                });
            } else {
                super.onDownloadDone(context, uri);
            }
        }
    }
}
