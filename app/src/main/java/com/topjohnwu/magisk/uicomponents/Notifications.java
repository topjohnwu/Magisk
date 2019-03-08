package com.topjohnwu.magisk.uicomponents;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.core.app.TaskStackBuilder;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.components.GeneralReceiver;
import com.topjohnwu.magisk.utils.Utils;

public class Notifications {

    public static NotificationManagerCompat mgr = NotificationManagerCompat.from(App.self);

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
        App app = App.self;

        Intent intent = new Intent(app, ClassMap.get(SplashActivity.class));
        intent.putExtra(Const.Key.OPEN_SECTION, "magisk");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(app);
        stackBuilder.addParentStack(ClassMap.get(SplashActivity.class));
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(app, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(app.getString(R.string.magisk_update_title))
                .setContentText(app.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        mgr.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void managerUpdate() {
        App app = App.self;
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Config.remoteManagerVersionString, Config.remoteManagerVersionCode);

        Intent intent = new Intent(app, ClassMap.get(GeneralReceiver.class));
        intent.setAction(Const.Key.BROADCAST_MANAGER_UPDATE);
        intent.putExtra(Const.Key.INTENT_SET_LINK, Config.managerLink);
        intent.putExtra(Const.Key.INTENT_SET_NAME, name);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(app,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(app, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(app.getString(R.string.manager_update_title))
                .setContentText(app.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        mgr.notify(Const.ID.APK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void dtboPatched() {
        App app = App.self;

        Intent intent = new Intent(app, ClassMap.get(GeneralReceiver.class))
                .setAction(Const.Key.BROADCAST_REBOOT);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(app,
                Const.ID.DTBO_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(app, Const.ID.UPDATE_NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(app.getString(R.string.dtbo_patched_title))
                .setContentText(app.getString(R.string.dtbo_patched_reboot))
                .setVibrate(new long[]{0, 100, 100, 100})
                .addAction(R.drawable.ic_refresh, app.getString(R.string.reboot), pendingIntent);

        mgr.notify(Const.ID.DTBO_NOTIFICATION_ID, builder.build());
    }

    public static NotificationCompat.Builder progress(String title) {
        App app = App.self;
        NotificationCompat.Builder builder = new NotificationCompat.Builder(app, Const.ID.PROGRESS_NOTIFICATION_CHANNEL);
        builder.setPriority(NotificationCompat.PRIORITY_LOW)
                .setSmallIcon(android.R.drawable.stat_sys_download)
                .setContentTitle(title)
                .setProgress(0, 0, true)
                .setOngoing(true);
        return builder;
    }
}
