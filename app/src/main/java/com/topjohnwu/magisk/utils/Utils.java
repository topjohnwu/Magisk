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
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Environment;
import android.provider.OpenableColumns;
import android.support.annotation.StringRes;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;

import java.io.File;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

public class Utils {

    public static boolean isDownloading = false;

    private static final int MAGISK_UPDATE_NOTIFICATION_ID = 1;
    private static final int APK_UPDATE_NOTIFICATION_ID = 2;

    public static boolean itemExist(Shell shell, String path) {
        String command = "[ -e " + path + " ] && echo true || echo false";
        List<String> ret = shell.su(command);
        return isValidShellResponse(ret) && Boolean.parseBoolean(ret.get(0));
    }

    public static void createFile(Shell shell, String path) {
        String folder = path.substring(0, path.lastIndexOf('/'));
        String command = "mkdir -p " + folder + " 2>/dev/null; touch " + path + " 2>/dev/null;";
        shell.su_raw(command);
    }

    public static void removeItem(Shell shell, String path) {
        String command = "rm -rf " + path + " 2>/dev/null";
        shell.su_raw(command);
    }

    public static List<String> getModList(Shell shell, String path) {
        String command = "ls -d " + path + "/* | grep -v lost+found";
        return shell.su(command);
    }

    public static List<String> readFile(Shell shell, String path) {
        String command = "cat " + path + " | sed '$a\\ ' | sed '$d'";
        return shell.su(command);
    }

    public static void dlAndReceive(Context context, DownloadReceiver receiver, String link, String filename) {
        if (isDownloading)
            return;

        runWithPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE, () -> {
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
        });
    }

    public static String getLegalFilename(CharSequence filename) {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("(", "").replace(")", "")
                .replace("#", "").replace("@", "").replace("*", "");
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

    public static int getPrefsInt(SharedPreferences prefs, String key) {
        return getPrefsInt(prefs, key, 0);
    }

    public static MagiskManager getMagiskManager(Context context) {
        return (MagiskManager) context.getApplicationContext();
    }

    public static void checkSafetyNet(FragmentActivity activity) {
        new SafetyNetHelper(activity) {
            @Override
            public void handleResults(Result result) {
                getMagiskManager(mActivity).SNCheckResult = result;
                getMagiskManager(mActivity).safetyNetDone.publish(false);
            }
        }.requestTest();
    }

    public static void clearRepoCache(Context context) {
        MagiskManager magiskManager = getMagiskManager(context);
        magiskManager.prefs.edit().remove(UpdateRepos.ETAG_KEY).apply();
        magiskManager.repoDB.clearRepo();
        magiskManager.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
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
        NotificationCompat.Builder builder = new NotificationCompat.Builder(magiskManager, MagiskManager.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(magiskManager.getString(R.string.magisk_update_title))
                .setContentText(magiskManager.getString(R.string.magisk_update_available, magiskManager.remoteMagiskVersionString))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(magiskManager, SplashActivity.class);
        intent.putExtra(MagiskManager.INTENT_SECTION, "magisk");
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
        NotificationCompat.Builder builder = new NotificationCompat.Builder(magiskManager, MagiskManager.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(magiskManager.getString(R.string.manager_update_title))
                .setContentText(magiskManager.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(magiskManager, ManagerUpdate.class);
        intent.putExtra(MagiskManager.INTENT_LINK, magiskManager.managerLink);
        intent.putExtra(MagiskManager.INTENT_VERSION, magiskManager.remoteManagerVersionString);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(magiskManager,
                APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) magiskManager.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(APK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void enableMagiskHide(Shell shell) {
        shell.su_raw("magiskhide --enable");
    }

    public static void disableMagiskHide(Shell shell) {
        shell.su_raw("magiskhide --disable");
    }

    public static List<String> listMagiskHide(Shell shell) {
        return shell.su("magiskhide --ls");
    }

    public static void addMagiskHide(Shell shell, String pkg) {
        shell.su_raw("magiskhide --add " + pkg);
    }

    public static void rmMagiskHide(Shell shell, String pkg) {
        shell.su_raw("magiskhide --rm " + pkg);
    }

    public static String getLocaleString(Context context, Locale locale, @StringRes int id) {
        Configuration config = context.getResources().getConfiguration();
        config.setLocale(locale);
        Context localizedContext = context.createConfigurationContext(config);
        return localizedContext.getString(id);
    }

    public static List<Locale> getAvailableLocale(Context context) {
        List<Locale> locales = new ArrayList<>();
        HashSet<String> set = new HashSet<>();
        Locale locale;

        int compareId = R.string.download_file_error;

        // Add default locale
        locales.add(Locale.ENGLISH);
        set.add(getLocaleString(context, Locale.ENGLISH, compareId));

        // Add some special locales
        locales.add(Locale.TAIWAN);
        set.add(getLocaleString(context, Locale.TAIWAN, compareId));
        locale = new Locale("pt", "BR");
        locales.add(locale);
        set.add(getLocaleString(context, locale, compareId));

        // Other locales
        for (String s : context.getAssets().getLocales()) {
            locale = Locale.forLanguageTag(s);
            if (set.add(getLocaleString(context, locale, compareId))) {
                locales.add(locale);
            }
        }

        Collections.sort(locales, (l1, l2) -> l1.getDisplayName(l1).compareTo(l2.getDisplayName(l2)));

        return locales;
    }

    public static String genPackageName(String prefix, int length) {
        StringBuilder builder = new StringBuilder(length);
        builder.append(prefix);
        length -= prefix.length();
        SecureRandom random = new SecureRandom();
        String base = "abcdefghijklmnopqrstuvwxyz";
        String alpha = base + base.toUpperCase();
        String full = alpha + "0123456789..........";
        char next, prev = '\0';
        for (int i = 0; i < length; ++i) {
            if (prev == '.' || i == length - 1 || i == 0) {
                next = alpha.charAt(random.nextInt(alpha.length()));
            } else {
                next = full.charAt(random.nextInt(full.length()));
            }
            builder.append(next);
            prev = next;
        }
        return builder.toString();
    }

    public static void runWithPermission(Context context, String permission, Runnable callback) {
        if (ContextCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
            // Passed in context should be an activity if not granted, need to show dialog!
            if (!(context instanceof com.topjohnwu.magisk.components.Activity))
                return;
            com.topjohnwu.magisk.components.Activity activity = (com.topjohnwu.magisk.components.Activity) context;
            activity.setPermissionGrantCallback(callback);
            ActivityCompat.requestPermissions(activity, new String[] { permission }, 0);
        } else {
            callback.run();
        }
    }
}