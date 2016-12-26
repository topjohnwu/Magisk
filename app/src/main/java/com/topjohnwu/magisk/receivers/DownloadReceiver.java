package com.topjohnwu.magisk.receivers;

import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.R;

public abstract class DownloadReceiver extends BroadcastReceiver {
    public Context mContext;
    public String mFilename;
    long downloadID;

    public DownloadReceiver() {}

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
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
                        onDownloadDone(uri);
                        break;
                    default:
                        Toast.makeText(context, R.string.download_file_error, Toast.LENGTH_LONG).show();
                        break;
                }
                context.unregisterReceiver(this);
            }
            c.close();
        }
    }

    public void setDownloadID(long id) {
        downloadID = id;
    }

    public void setFilename(String filename) {
        mFilename = filename;
    }

    public abstract void onDownloadDone(Uri uri);
}
