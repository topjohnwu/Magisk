package com.topjohnwu.magisk.receivers;

import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

public abstract class DownloadReceiver extends BroadcastReceiver {
    protected File mFile;
    private long downloadID;

    @Override
    public void onReceive(Context context, Intent intent) {
        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        String action = intent.getAction();
        if (DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(action)) {
            DownloadManager.Query query = new DownloadManager.Query();
            query.setFilterById(downloadID);
            Cursor c = downloadManager.query(query);
            if (c.moveToFirst()) {
                int columnIndex = c.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int status = c.getInt(columnIndex);
                switch (status) {
                    case DownloadManager.STATUS_SUCCESSFUL:
                        Uri uri = Uri.parse(c.getString(c.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI)));
                        onDownloadDone(context, uri);
                        break;
                    default:
                        MagiskManager.toast(R.string.download_file_error, Toast.LENGTH_LONG);
                        break;
                }
                context.unregisterReceiver(this);
            }
            c.close();
        }
        Utils.isDownloading = false;
    }

    public DownloadReceiver setDownloadID(long id) {
        downloadID = id;
        return this;
    }

    public DownloadReceiver setFile(File file) {
        mFile = file;
        return this;
    }

    public abstract void onDownloadDone(Context context, Uri uri);
}
