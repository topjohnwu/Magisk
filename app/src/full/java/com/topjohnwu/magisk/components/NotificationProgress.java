package com.topjohnwu.magisk.components;

import android.widget.Toast;

import com.androidnetworking.interfaces.DownloadProgressListener;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Notifications;
import com.topjohnwu.magisk.utils.Utils;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

public class NotificationProgress implements DownloadProgressListener {

    private NotificationManagerCompat mgr;
    private NotificationCompat.Builder builder;
    private long prevTime;

    public NotificationProgress(String title) {
        MagiskManager mm = Data.MM();
        mgr = NotificationManagerCompat.from(mm);
        builder = Notifications.progress(title);
        mgr.notify(Const.ID.DOWNLOAD_PROGRESS_ID, builder.build());
        prevTime = System.currentTimeMillis();
        Utils.toast(mm.getString(R.string.downloading_toast, title), Toast.LENGTH_SHORT);
    }

    @Override
    public void onProgress(long bytesDownloaded, long totalBytes) {
        long cur = System.currentTimeMillis();
        if (cur - prevTime >= 1000) {
            prevTime = cur;
            builder.setProgress((int) totalBytes, (int) bytesDownloaded, false);
            builder.setContentText(bytesDownloaded * 100 / totalBytes + "%");
            update();
        }
    }

    public NotificationCompat.Builder getBuilder() {
        return builder;
    }

    public void update() {
        mgr.notify(Const.ID.DOWNLOAD_PROGRESS_ID, builder.build());
    }

    public void dlDone() {
        builder.setProgress(0, 0, false);
        builder.setContentText(Data.MM().getString(R.string.download_complete));
        update();
    }

    public void dismiss() {
        mgr.cancel(Const.ID.DOWNLOAD_PROGRESS_ID);
    }
}
