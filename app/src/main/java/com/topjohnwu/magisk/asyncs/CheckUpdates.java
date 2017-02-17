package com.topjohnwu.magisk.asyncs;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.support.v4.app.TaskStackBuilder;
import android.support.v7.app.NotificationCompat;

import com.topjohnwu.magisk.MainActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates extends ParallelTask<Void, Void, Void> {

    private static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";
    private static final int NOTIFICATION_ID = 1;

    private boolean showNotification = false;

    public CheckUpdates(Context context, boolean b) {
        this(context);
        showNotification = b;
    }

    public CheckUpdates(Context context) {
        magiskManager = Utils.getMagiskManager(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        String jsonStr = WebService.request(UPDATE_JSON, WebService.GET);
        try {
            JSONObject json = new JSONObject(jsonStr);
            JSONObject magisk = json.getJSONObject("magisk");
            magiskManager.remoteMagiskVersion = magisk.getDouble("versionCode");
            magiskManager.magiskLink = magisk.getString("link");
            magiskManager.releaseNoteLink = magisk.getString("note");
        } catch (JSONException ignored) {
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        if (magiskManager.magiskVersion < magiskManager.remoteMagiskVersion && showNotification) {
            NotificationCompat.Builder builder = new NotificationCompat.Builder(magiskManager);
            builder.setSmallIcon(R.drawable.ic_magisk)
                    .setContentTitle(magiskManager.getString(R.string.magisk_update_title))
                    .setContentText(magiskManager.getString(R.string.magisk_update_available, magiskManager.remoteMagiskVersion))
                    .setVibrate(new long[]{0, 100, 100, 100})
                    .setAutoCancel(true);
            Intent intent = new Intent(magiskManager, SplashActivity.class);
            intent.putExtra(MainActivity.SECTION, "install");
            TaskStackBuilder stackBuilder = TaskStackBuilder.create(magiskManager);
            stackBuilder.addParentStack(MainActivity.class);
            stackBuilder.addNextIntent(intent);
            PendingIntent pendingIntent = stackBuilder.getPendingIntent(NOTIFICATION_ID, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(pendingIntent);
            NotificationManager notificationManager =
                    (NotificationManager) magiskManager.getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.notify(NOTIFICATION_ID, builder.build());
        }
        magiskManager.updateCheckDone.trigger();
    }
}
