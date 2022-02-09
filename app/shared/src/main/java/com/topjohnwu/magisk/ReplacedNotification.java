package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class ReplacedNotification {
    private static final String REPLACED_NOTIFICATION_CHANNEL = "replaced";
    private static final int REPLACED_NOTIFICATION_ID = 4;

    public static void show(Context context, String channelName, String title, String text) {
        var pm = context.getPackageManager();
        var intent = pm.getLaunchIntentForPackage(context.getPackageName());
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        // noinspection InlinedApi
        var flag = PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT;
        var pending = PendingIntent.getActivity(context, 0, intent, flag);
        var nm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        var builder = new Notification.Builder(context);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            var channel = new NotificationChannel(REPLACED_NOTIFICATION_CHANNEL,
                    channelName, NotificationManager.IMPORTANCE_HIGH);
            nm.createNotificationChannel(channel);
            builder.setChannelId(REPLACED_NOTIFICATION_CHANNEL);
        } else {
            builder.setPriority(Notification.PRIORITY_HIGH);
        }
        var notification = builder
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentIntent(pending)
                .setContentTitle(title)
                .setContentText(text)
                .setAutoCancel(true)
                .build();
        nm.notify(REPLACED_NOTIFICATION_ID, notification);
    }

    public static void restartApplication(Activity activity) {
        var pm = activity.getPackageManager();
        var intent = pm.getLaunchIntentForPackage(activity.getPackageName());
        activity.finishAffinity();
        activity.startActivity(intent);
        Runtime.getRuntime().exit(0);
    }
}
