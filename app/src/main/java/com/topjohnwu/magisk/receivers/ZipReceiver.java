package com.topjohnwu.magisk.receivers;

import android.support.v7.app.AlertDialog;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

public class ZipReceiver extends DownloadReceiver {
    @Override
    public void task(File file) {
        new AlertDialog.Builder(mContext)
                .setTitle("Installation")
                .setMessage("Do you want to install now?")
                .setCancelable(false)
                .setPositiveButton("Yes, flash now", (dialogInterface, i) -> {
                    new Utils.flashZIP(file.getPath(), mContext).execute();
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }
}
