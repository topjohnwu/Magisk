package com.topjohnwu.magisk.utils;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.services.UpdateCheckService;

import java.util.Locale;

public class Utils {

    public static int getPrefsInt(SharedPreferences prefs, String key, int def) {
        return Integer.parseInt(prefs.getString(key, String.valueOf(def)));
    }

    public static int getPrefsInt(SharedPreferences prefs, String key) {
        return getPrefsInt(prefs, key, 0);
    }

    public static String getNameFromUri(Context context, Uri uri) {
        String name = null;
        try (Cursor c = context.getContentResolver().query(uri, null, null, null, null)) {
            if (c != null) {
                int nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (nameIndex != -1) {
                    c.moveToFirst();
                    name = c.getString(nameIndex);
                }
            }
        }
        if (name == null) {
            int idx = uri.getPath().lastIndexOf('/');
            name = uri.getPath().substring(idx + 1);
        }
        return name;
    }

    public static int dpInPx(int dp) {
        float scale = Data.MM().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5);
    }

    public static String fmt(String fmt, Object... args) {
        return String.format(Locale.US, fmt, args);
    }

    public static String dos2unix(String s) {
        String newString = s.replace("\r\n", "\n");
        if(!newString.endsWith("\n")) {
            return newString + "\n";
        } else {
            return newString;
        }
    }

    public static void setupUpdateCheck() {
        MagiskManager mm = Data.MM();
        JobScheduler scheduler = (JobScheduler) mm.getSystemService(Context.JOB_SCHEDULER_SERVICE);

        if (mm.prefs.getBoolean(Const.Key.CHECK_UPDATES, true)) {
            if (scheduler.getAllPendingJobs().isEmpty() ||
                    Const.UPDATE_SERVICE_VER > mm.prefs.getInt(Const.Key.UPDATE_SERVICE_VER, -1)) {
                ComponentName service = new ComponentName(mm, UpdateCheckService.class);
                JobInfo info = new JobInfo.Builder(Const.ID.UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                scheduler.schedule(info);
            }
        } else {
            scheduler.cancel(Const.UPDATE_SERVICE_VER);
        }
    }

    public static void toast(CharSequence msg, int duration) {
        Data.mainHandler.post(() -> Toast.makeText(Data.MM(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        Data.mainHandler.post(() -> Toast.makeText(Data.MM(), resId, duration).show());
    }
}