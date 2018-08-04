package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.asyncs.PatchAPK;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.utils.JarMap;
import com.topjohnwu.utils.SignAPK;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;

public class ManagerUpdate extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Download.receive(
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
                    String orig = uri.getPath();
                    String patch = orig.substring(0, orig.lastIndexOf('.')) + "-patched.apk";
                    try {
                        JarMap apk = new JarMap(orig);
                        PatchAPK.patchPackageID(apk, Const.ORIG_PKG_NAME, context.getPackageName());
                        SignAPK.sign(apk, new BufferedOutputStream(new FileOutputStream(patch)));
                        super.onDownloadDone(context, Uri.fromFile(new File(patch)));
                    } catch (Exception ignored) { }
                });
            } else {
                super.onDownloadDone(context, uri);
            }
        }
    }
}
