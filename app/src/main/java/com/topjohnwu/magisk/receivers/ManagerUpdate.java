package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.support.v4.content.FileProvider;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

public class ManagerUpdate extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager magiskManager = Utils.getMagiskManager(context);
        Utils.dlAndReceive(
                magiskManager,
                new DownloadReceiver() {
                    @Override
                    public void onDownloadDone(Uri uri) {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                            Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                            install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                            Uri content = FileProvider.getUriForFile(mContext,
                                    "com.topjohnwu.magisk.provider", new File(uri.getPath()));
                            install.setData(content);
                            mContext.startActivity(install);
                        } else {
                            Intent install = new Intent(Intent.ACTION_VIEW);
                            install.setDataAndType(uri, "application/vnd.android.package-archive");
                            install.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            mContext.startActivity(install);
                        }
                    }
                },
                magiskManager.managerLink,
                Utils.getLegalFilename("MagiskManager-v" +
                        magiskManager.remoteManagerVersionString + ".apk"));
    }
}
