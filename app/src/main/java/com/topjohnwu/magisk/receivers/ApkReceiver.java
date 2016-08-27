package com.topjohnwu.magisk.receivers;

import android.content.Intent;
import android.support.v4.content.FileProvider;

import java.io.File;

public class ApkReceiver extends DownloadReceiver {

    @Override
    public void task(File file) {
        Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
        install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        install.setData(FileProvider.getUriForFile(mContext, "com.topjohnwu.magisk.provider", file));
        mContext.startActivity(install);
    }
}
