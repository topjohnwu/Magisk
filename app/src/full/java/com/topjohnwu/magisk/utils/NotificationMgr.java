package com.topjohnwu.magisk.utils;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.receivers.ManagerUpdate;
import com.topjohnwu.magisk.receivers.RebootReceiver;

import androidx.core.app.NotificationCompat;
import androidx.core.app.TaskStackBuilder;

public class NotificationMgr {

    public static void magiskUpdate() {
        MagiskManager mm = Data.MM();

        Intent intent = new Intent(mm, Data.classMap.get(SplashActivity.class));
        intent.putExtra(Const.Key.OPEN_SECTION, "magisk");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(mm);
        stackBuilder.addParentStack(Data.classMap.get(SplashActivity.class));
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.magisk_update_title))
                .setContentText(mm.getString(R.string.magisk_update_available, Data.remoteMagiskVersionString))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void managerUpdate() {
        MagiskManager mm = Data.MM();
        String filename = Utils.fmt("MagiskManager-v%s(%d).apk",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);

        Intent intent = new Intent(mm, Data.classMap.get(ManagerUpdate.class));
        intent.putExtra(Const.Key.INTENT_SET_LINK, Data.managerLink);
        intent.putExtra(Const.Key.INTENT_SET_FILENAME, filename);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.manager_update_title))
                .setContentText(mm.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(Const.ID.APK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void dtboPatched() {
        MagiskManager mm = Data.MM();

        Intent intent = new Intent(mm, Data.classMap.get(RebootReceiver.class));
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                Const.ID.DTBO_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.dtbo_patched_title))
                .setContentText(mm.getString(R.string.dtbo_patched_reboot))
                .setVibrate(new long[]{0, 100, 100, 100})
                .addAction(R.drawable.ic_refresh, mm.getString(R.string.reboot), pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(Const.ID.DTBO_NOTIFICATION_ID, builder.build());
    }
}
