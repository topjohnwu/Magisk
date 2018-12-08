package com.topjohnwu.magisk.components;

import android.widget.Toast;

import com.androidnetworking.interfaces.DownloadProgressListener;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

public class ProgressNotification implements DownloadProgressListener {

    private NotificationManagerCompat mgr;
    private NotificationCompat.Builder builder;
    private long prevTime;

    public ProgressNotification(String title) {
        MagiskManager mm = Data.MM();
        mgr = NotificationManagerCompat.from(mm);
        builder = Notifications.progress(title);
        prevTime = System.currentTimeMillis();
        update();
        Utils.toast(mm.getString(R.string.downloading_toast, title), Toast.LENGTH_SHORT);
    }

    @Override
    public void onProgress(long bytesDownloaded, long totalBytes) {
        long cur = System.currentTimeMillis();
        if (cur - prevTime >= 1000) {
            prevTime = cur;
            int progress = (int) (bytesDownloaded * 100 / totalBytes);
            builder.setProgress(100, progress, false);
            builder.setContentText(progress + "%");
            update();
        }
    }

    public NotificationCompat.Builder getNotification() {
        return builder;
    }

    public void update() {
        mgr.notify(hashCode(), builder.build());
    }

    public void dlDone() {
        builder.setProgress(0, 0, false)
                .setContentText(Data.MM().getString(R.string.download_complete))
                .setSmallIcon(R.drawable.ic_check_circle)
                .setOngoing(false);
        update();
    }

    public void dlFail() {
        builder.setProgress(0, 0, false)
                .setContentText(Data.MM().getString(R.string.download_file_error))
                .setSmallIcon(R.drawable.ic_cancel)
                .setOngoing(false);
        update();
    }

    public void dismiss() {
        mgr.cancel(hashCode());
    }
}
