package com.topjohnwu.magisk.uicomponents;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Intent;
import android.widget.Toast;

import androidx.core.app.NotificationCompat;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.DownloadProgressListener;

public class ProgressNotification implements DownloadProgressListener {

    private NotificationCompat.Builder builder;
    private Notification notification;
    private long prevTime;

    public ProgressNotification(String title) {
        builder = Notifications.progress(title);
        prevTime = System.currentTimeMillis();
        update();
        Utils.toast(App.self.getString(R.string.downloading_toast, title), Toast.LENGTH_SHORT);
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

    public NotificationCompat.Builder getNotificationBuilder() {
        return builder;
    }

    public Notification getNotification() {
        return notification;
    }

    public void update() {
        notification = builder.build();
        Notifications.mgr.notify(hashCode(), notification);
    }

    private void lastUpdate() {
        notification = builder.build();
        Notifications.mgr.cancel(hashCode());
        Notifications.mgr.notify(notification.hashCode(), notification);
    }

    public void dlDone() {
        dlDone(PendingIntent.getActivity(App.self, hashCode(),
                new Intent(), PendingIntent.FLAG_UPDATE_CURRENT));
    }

    public void dlDone(PendingIntent intent) {
        builder.setProgress(0, 0, false)
                .setContentText(App.self.getString(R.string.download_complete))
                .setSmallIcon(android.R.drawable.stat_sys_download_done)
                .setContentIntent(intent)
                .setOngoing(false)
                .setAutoCancel(true);
        lastUpdate();
    }

    public void dlFail() {
        builder.setProgress(0, 0, false)
                .setContentText(App.self.getString(R.string.download_file_error))
                .setSmallIcon(android.R.drawable.stat_notify_error)
                .setOngoing(false);
        lastUpdate();
    }

    public void dismiss() {
        Notifications.mgr.cancel(hashCode());
    }
}
