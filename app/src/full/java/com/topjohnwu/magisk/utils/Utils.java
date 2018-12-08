package com.topjohnwu.magisk.utils;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.OpenableColumns;
import android.widget.Toast;

import com.androidnetworking.AndroidNetworking;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.ValueSortedMap;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import java.util.Locale;
import java.util.Map;

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
                ComponentName service = new ComponentName(mm, Data.classMap.get(UpdateCheckService.class));
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

    public static void openLink(Context context, Uri link) {
        Intent intent = new Intent(Intent.ACTION_VIEW, link);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (intent.resolveActivity(context.getPackageManager()) != null) {
            context.startActivity(intent);
        } else {
            toast(R.string.open_link_failed_toast, Toast.LENGTH_SHORT);
        }
    }

    public static void toast(CharSequence msg, int duration) {
        Data.mainHandler.post(() -> Toast.makeText(Data.MM(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        Data.mainHandler.post(() -> Toast.makeText(Data.MM(), resId, duration).show());
    }

    public static void loadModules() {
        Topic.reset(Topic.MODULE_LOAD_DONE);
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            Map<String, Module> moduleMap = new ValueSortedMap<>();
            SuFile path = new SuFile(Const.MAGISK_PATH);
            SuFile[] modules = path.listFiles(
                    (file, name) -> !name.equals("lost+found") && !name.equals(".core"));
            for (SuFile file : modules) {
                if (file.isFile()) continue;
                Module module = new Module(Const.MAGISK_PATH + "/" + file.getName());
                moduleMap.put(module.getId(), module);
            }
            Topic.publish(Topic.MODULE_LOAD_DONE, moduleMap);
        });
    }

    public static String getAppLabel(ApplicationInfo info, PackageManager pm) {
        try {
            if (info.labelRes > 0) {
                Resources res = pm.getResourcesForApplication(info);
                Configuration config = new Configuration();
                config.setLocale(LocaleManager.locale);
                res.updateConfiguration(config, res.getDisplayMetrics());
                return res.getString(info.labelRes);
            }
        } catch (Exception ignored) {}
        return info.loadLabel(pm).toString();
    }

    public static boolean showSuperUser() {
        if (Data.multiuserState < 0)
            Data.multiuserState = Data.MM().mDB.getSettings(Const.Key.SU_MULTIUSER_MODE,
                    Const.Value.MULTIUSER_MODE_OWNER_ONLY);
        return Shell.rootAccess() && (Const.USER_ID == 0 ||
                Data.multiuserState != Const.Value.MULTIUSER_MODE_OWNER_MANAGED);
    }

    public static String dlString(String url) {
        String s = (String) AndroidNetworking.get(url).build().executeForString().getResult();
        return s == null ? "" : s;
    }
}