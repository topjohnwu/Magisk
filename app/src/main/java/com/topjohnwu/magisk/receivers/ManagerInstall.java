package com.topjohnwu.magisk.receivers;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.support.v4.content.FileProvider;

import java.io.File;

public class ManagerInstall extends DownloadReceiver {
    @Override
    public void onDownloadDone(Context context, Uri uri) {
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
}
