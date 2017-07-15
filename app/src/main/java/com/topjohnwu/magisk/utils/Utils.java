package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.DownloadManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Environment;
import android.provider.OpenableColumns;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.TaskStackBuilder;
import android.support.v7.app.NotificationCompat;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.LoadRepos;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;

import java.io.File;
import java.util.List;

public class Utils {

    public static boolean isDownloading = false;

    private static final int MAGISK_UPDATE_NOTIFICATION_ID = 1;
    private static final int APK_UPDATE_NOTIFICATION_ID = 2;

    public static boolean itemExist(Shell shell, String path) {
        String command = "if [ -e " + path + " ]; then echo true; else echo false; fi";
        List<String> ret = shell.su(command);
        return isValidShellResponse(ret) && Boolean.parseBoolean(ret.get(0));
    }

    public static void createFile(Shell shell, String path) {
        String folder = path.substring(0, path.lastIndexOf('/'));
        String command = "mkdir -p " + folder + " 2>/dev/null; touch " + path + " 2>/dev/null; if [ -f \"" + path + "\" ]; then echo true; else echo false; fi";
        shell.su_raw(command);
    }

    public static void removeItem(Shell shell, String path) {
        String command = "rm -rf " + path + " 2>/dev/null; if [ -e " + path + " ]; then echo false; else echo true; fi";
        shell.su_raw(command);
    }

    public static List<String> getModList(Shell shell, String path) {
        String command = "find " + path + " -type d -maxdepth 1 ! -name \"*.core\" ! -name \"*lost+found\" ! -name \"*magisk\"";
        return shell.su(command);
    }

    public static List<String> readFile(Shell shell, String path) {
        String command = "cat " + path;
        return shell.su(command);
    }

    public static void dlAndReceive(Context context, DownloadReceiver receiver, String link, String filename) {
        if (isDownloading)
            return;

        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }

        File file = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + filename);

        if ((!file.getParentFile().exists() && !file.getParentFile().mkdirs())
                || (file.exists() && !file.delete())) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }

        Toast.makeText(context, context.getString(R.string.downloading_toast, filename), Toast.LENGTH_LONG).show();
        isDownloading = true;

        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);

        if (link != null) {
            DownloadManager.Request request = new DownloadManager.Request(Uri.parse(link));
            request.setDestinationUri(Uri.fromFile(file));
            receiver.setDownloadID(downloadManager.enqueue(request));
        }
        receiver.setFilename(filename);
        context.getApplicationContext().registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    public static String getLegalFilename(CharSequence filename) {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("(", "").replace(")", "")
                .replace("#", "").replace("@", "").replace("*", "");
    }

    public static String detectBootImage(Shell shell) {
        String[] commands = {
                "for PARTITION in kern-a KERN-A android_boot ANDROID_BOOT kernel KERNEL boot BOOT lnx LNX; do",
                "BOOTIMAGE=`readlink /dev/block/by-name/$PARTITION || " +
                        "readlink /dev/block/platform/*/by-name/$PARTITION || " +
                        "readlink /dev/block/platform/*/*/by-name/$PARTITION`",
                "if [ ! -z \"$BOOTIMAGE\" ]; then break; fi",
                "done",
                "echo \"$BOOTIMAGE\""
        };
        List<String> ret = shell.su(commands);
        if (isValidShellResponse(ret)) {
            return ret.get(0);
        }
        return null;
    }

    public static boolean lowercaseContains(CharSequence string, CharSequence nonNullLowercaseSearch) {
        return !TextUtils.isEmpty(string) && string.toString().toLowerCase().contains(nonNullLowercaseSearch);
    }

    public static boolean isValidShellResponse(List<String> list) {
        if (list != null && list.size() != 0) {
            // Check if all empty
            for (String res : list) {
                if (!TextUtils.isEmpty(res)) return true;
            }
        }
        return false;
    }

    public static int getPrefsInt(SharedPreferences prefs, String key, int def) {
        return Integer.parseInt(prefs.getString(key, String.valueOf(def)));
    }

    public static MagiskManager getMagiskManager(Context context) {
        return (MagiskManager) context.getApplicationContext();
    }

    public static void checkSafetyNet(FragmentActivity activity) {
        new SafetyNetHelper(activity) {
            @Override
            public void handleResults(Result result) {
                getMagiskManager(mActivity).SNCheckResult = result;
                getMagiskManager(mActivity).safetyNetDone.trigger();
            }
        }.requestTest();
    }

    public static void clearRepoCache(Activity activity) {
        MagiskManager magiskManager = getMagiskManager(activity);
        magiskManager.prefs.edit().remove(LoadRepos.ETAG_KEY).apply();
        new RepoDatabaseHelper(activity).clearRepo();
        Toast.makeText(activity, R.string.repo_cache_cleared, Toast.LENGTH_SHORT).show();
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

    public static void showUriSnack(Activity activity, Uri uri) {
        SnackbarMaker.make(activity, activity.getString(R.string.internal_storage,
                "/MagiskManager/" + Utils.getNameFromUri(activity, uri)),
                Snackbar.LENGTH_LONG)
                .setAction(R.string.ok, (v)->{}).show();
    }

    public static boolean checkNetworkStatus(Context context) {
        ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = manager.getActiveNetworkInfo();
        return networkInfo != null && networkInfo.isConnected();
    }

    public static boolean checkBits(int bits, int... masks) {
        for (int mask : masks) {
            if ((bits & mask) == 0)
                return false;
        }
        return true;
    }

    public static void showMagiskUpdate(MagiskManager magiskManager) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(magiskManager);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(magiskManager.getString(R.string.magisk_update_title))
                .setContentText(magiskManager.getString(R.string.magisk_update_available, magiskManager.remoteMagiskVersionString))
                .setChannelId(MagiskManager.NOTIFICATION_CHANNEL)
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(magiskManager, SplashActivity.class);
        intent.putExtra(MagiskManager.INTENT_SECTION, "install");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(magiskManager);
        stackBuilder.addParentStack(SplashActivity.class);
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) magiskManager.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void showManagerUpdate(MagiskManager magiskManager) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(magiskManager);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(magiskManager.getString(R.string.manager_update_title))
                .setContentText(magiskManager.getString(R.string.manager_download_install))
                .setChannelId(MagiskManager.NOTIFICATION_CHANNEL)
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(magiskManager, ManagerUpdate.class);
        intent.putExtra("link", magiskManager.managerLink);
        intent.putExtra("version", magiskManager.remoteManagerVersionString);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(magiskManager,
                APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) magiskManager.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(APK_UPDATE_NOTIFICATION_ID, builder.build());
    }
}