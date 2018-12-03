package com.topjohnwu.magisk.components;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.receivers.GeneralReceiver;
import com.topjohnwu.magisk.utils.Utils;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.core.app.TaskStackBuilder;

public class Notifications {

    public static void setup(Context c) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationManager mgr = c.getSystemService(NotificationManager.class);
            mgr.deleteNotificationChannel("magisk_notification");
            NotificationChannel channel =
                    new NotificationChannel(Const.ID.UPDATE_NOTIFICATION_CHANNEL,
                    c.getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT);
            mgr.createNotificationChannel(channel);
            channel = new NotificationChannel(Const.ID.PROGRESS_NOTIFICATION_CHANNEL,
                    c.getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW);
            mgr.createNotificationChannel(channel);
        }
    }

    public static void magiskUpdate() {
        MagiskManager mm = Data.MM();

        Intent intent = new Intent(mm, Data.classMap.get(SplashActivity.class));
        intent.putExtra(Const.Key.OPEN_SECTION, "magisk");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(mm);
        stackBuilder.addParentStack(Data.classMap.get(SplashActivity.class));
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.magisk_update_title))
                .setContentText(mm.getString(R.string.magisk_update_available, Data.remoteMagiskVersionString))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        NotificationManagerCompat mgr = NotificationManagerCompat.from(mm);
        mgr.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void managerUpdate() {
        MagiskManager mm = Data.MM();
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);

        Intent intent = new Intent(mm, Data.classMap.get(GeneralReceiver.class));
        intent.setAction(Const.Key.BROADCAST_MANAGER_UPDATE);
        intent.putExtra(Const.Key.INTENT_SET_LINK, Data.managerLink);
        intent.putExtra(Const.Key.INTENT_SET_NAME, name);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.manager_update_title))
                .setContentText(mm.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        NotificationManagerCompat mgr = NotificationManagerCompat.from(mm);
        mgr.notify(Const.ID.APK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void dtboPatched() {
        MagiskManager mm = Data.MM();

        Intent intent = new Intent(mm, Data.classMap.get(GeneralReceiver.class))
                .setAction(Const.Key.BROADCAST_REBOOT);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                Const.ID.DTBO_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.dtbo_patched_title))
                .setContentText(mm.getString(R.string.dtbo_patched_reboot))
                .setVibrate(new long[]{0, 100, 100, 100})
                .addAction(R.drawable.ic_refresh, mm.getString(R.string.reboot), pendingIntent);

        NotificationManagerCompat mgr = NotificationManagerCompat.from(mm);
        mgr.notify(Const.ID.DTBO_NOTIFICATION_ID, builder.build());
    }

    public static NotificationCompat.Builder progress(String title) {
        MagiskManager mm = Data.MM();
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.PROGRESS_NOTIFICATION_CHANNEL);
        builder.setPriority(NotificationCompat.PRIORITY_LOW)
                .setSmallIcon(android.R.drawable.stat_sys_download)
                .setContentTitle(title)
                .setProgress(0, 0, true)
                .setOngoing(true);
        return builder;
    }
}
