package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.support.v4.content.FileProvider;

import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

public class ManagerUpdate extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Utils.dlAndReceive(
                context,
                new DownloadReceiver() {
                    @Override
                    public void onDownloadDone(Uri uri) {
                        if (!context.getPackageName().equals(Const.ORIG_PKG_NAME)) {
                            Utils.dumpPrefs();
                        }
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                            Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                            install.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                            install.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            Uri content = FileProvider.getUriForFile(context,
                                    context.getPackageName() + ".provider", new File(uri.getPath()));
                            install.setData(content);
                            context.startActivity(install);
                        } else {
                            Intent install = new Intent(Intent.ACTION_VIEW);
                            install.setDataAndType(uri, "application/vnd.android.package-archive");
                            install.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            context.startActivity(install);
                        }
                    }
                },
                intent.getStringExtra(Const.Key.INTENT_SET_LINK),
                Utils.fmt("MagiskManager-v%s.apk", intent.getStringExtra(Const.Key.INTENT_SET_VERSION))
        );
    }
}
