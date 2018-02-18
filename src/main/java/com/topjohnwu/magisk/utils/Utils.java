package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.DownloadManager;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.support.annotation.StringRes;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.widget.Toast;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class Utils {

    public static boolean isDownloading = false;

    public static String cmd(String cmd) {
        return ShellUtils.fastCmd(Shell.getShell(), cmd);
    }

    public static void uninstallPkg(String pkg) {
        Shell.Sync.su("sudb_clean " + Const.USER_ID, "pm uninstall " + pkg);
    }

    public static void dlAndReceive(Context context, DownloadReceiver receiver, String link, String filename) {
        if (isDownloading)
            return;

        runWithPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE, () -> {
            File file = new File(Const.EXTERNAL_PATH, getLegalFilename(filename));

            if ((!file.getParentFile().exists() && !file.getParentFile().mkdirs())
                    || (file.exists() && !file.delete())) {
                Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
                return;
            }

            Toast.makeText(context, context.getString(R.string.downloading_toast, filename), Toast.LENGTH_LONG).show();
            isDownloading = true;

            DownloadManager.Request request = new DownloadManager
                    .Request(Uri.parse(link))
                    .setDestinationUri(Uri.fromFile(file));

            DownloadManager dm = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
            receiver.setDownloadID(dm.enqueue(request)).setFile(file);
            context.getApplicationContext().registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
        });
    }

    public static String getLegalFilename(CharSequence filename) {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("*", "").replace("/", "_")
                .replace("#", "").replace("@", "").replace("\\", "_");
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

    public static boolean checkNetworkStatus() {
        ConnectivityManager manager = (ConnectivityManager)
                MagiskManager.get().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = manager.getActiveNetworkInfo();
        return networkInfo != null && networkInfo.isConnected();
    }

    public static String getLocaleString(Locale locale, @StringRes int id) {
        Context context = MagiskManager.get();
        Configuration config = context.getResources().getConfiguration();
        config.setLocale(locale);
        Context localizedContext = context.createConfigurationContext(config);
        return localizedContext.getString(id);
    }

    public static List<Locale> getAvailableLocale() {
        List<Locale> locales = new ArrayList<>();
        HashSet<String> set = new HashSet<>();
        Locale locale;

        @StringRes int compareId = R.string.download_file_error;

        // Add default locale
        locales.add(Locale.ENGLISH);
        set.add(getLocaleString(Locale.ENGLISH, compareId));

        // Add some special locales
        locales.add(Locale.TAIWAN);
        set.add(getLocaleString(Locale.TAIWAN, compareId));
        locale = new Locale("pt", "BR");
        locales.add(locale);
        set.add(getLocaleString(locale, compareId));

        // Other locales
        for (String s : MagiskManager.get().getAssets().getLocales()) {
            locale = Locale.forLanguageTag(s);
            if (set.add(getLocaleString(locale, compareId))) {
                locales.add(locale);
            }
        }

        Collections.sort(locales, (l1, l2) -> l1.getDisplayName(l1).compareTo(l2.getDisplayName(l2)));

        return locales;
    }

    public static void runWithPermission(Context context, String permission, Runnable callback) {
        if (ContextCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
            // Passed in context should be an activity if not granted, need to show dialog!
            Utils.getMagiskManager(context).setPermissionGrantCallback(callback);
            if (!(context instanceof com.topjohnwu.magisk.components.Activity)) {
                // Start activity to show dialog
                Intent intent = new Intent(context, SplashActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.putExtra(Const.Key.INTENT_PERM, permission);
                context.startActivity(intent);
            } else {
                ActivityCompat.requestPermissions((Activity) context, new String[] { permission }, 0);
            }

        } else {
            callback.run();
        }
    }

    public static AssetManager getAssets(String apk) {
        try {
            AssetManager asset = AssetManager.class.newInstance();
            AssetManager.class.getMethod("addAssetPath", String.class).invoke(asset, apk);
            return asset;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void patchDTBO() {
        MagiskManager mm = MagiskManager.get();
        if (mm.magiskVersionCode >= 1446 && !mm.keepVerity) {
            if (ShellUtils.fastCmdResult(Shell.getShell(), "patch_dtbo_image")) {
                ShowUI.dtboPatchedNotification();
            }
        }
    }

    public static int dpInPx(int dp) {
        Context context = MagiskManager.get();
        float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5);
    }

    public static void dumpPrefs() {
        Gson gson = new Gson();
        Map<String, ?> prefs = MagiskManager.get().prefs.getAll();
        prefs.remove("App Restrictions");
        String json = gson.toJson(prefs, new TypeToken<Map<String, ?>>(){}.getType());
        Shell.Sync.su(fmt("for usr in /data/user/*; do echo '%s' > ${usr}/%s; done", json, Const.MANAGER_CONFIGS));
    }

    public static void loadPrefs() {
        SuFile config = new SuFile(fmt("/data/user/%d/%s", Const.USER_ID, Const.MANAGER_CONFIGS), true);
        List<String> ret = Shell.Sync.su("cat " + config);
        if (ShellUtils.isValidOutput(ret)) {
            SharedPreferences.Editor editor = MagiskManager.get().prefs.edit();
            String json = ret.get(0);
            Gson gson = new Gson();
            Map<String, ?> prefMap = gson.fromJson(json, new TypeToken<Map<String, ?>>(){}.getType());
            editor.clear();
            for (Map.Entry<String, ?> entry : prefMap.entrySet()) {
                Object value = entry.getValue();
                if (value instanceof String) {
                    editor.putString(entry.getKey(), (String) value);
                } else if (value instanceof Boolean) {
                    editor.putBoolean(entry.getKey(), (boolean) value);
                } else if (value instanceof Number) {
                    editor.putInt(entry.getKey(), ((Number) value).intValue());
                }
            }
            editor.remove(Const.Key.ETAG_KEY);
            editor.apply();
            MagiskManager.get().loadConfig();
            config.delete();
        }
    }

    public static String fmt(String fmt, Object... args) {
        return String.format(Locale.US, fmt, args);
    }
}