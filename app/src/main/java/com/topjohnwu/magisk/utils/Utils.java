package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.DownloadManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.admin.DevicePolicyManager;
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
import android.os.Build;
import android.os.Environment;
import android.provider.OpenableColumns;
import android.support.annotation.StringRes;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskFragment;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.RestoreStockBoot;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

public class Utils {

    public static final int SELECT_BOOT_IMG = 3;
    public static final String UNINSTALLER = "magisk_uninstaller.sh";
    public static final String UTIL_FUNCTIONS= "util_functions.sh";
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
        MagiskManager mm = getMagiskManager(context);
        mm.prefs.edit().remove(UpdateRepos.ETAG_KEY).apply();
        mm.repoDB.clearRepo();
        mm.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
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

    public static void showMagiskUpdateNotification(MagiskManager mm) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, MagiskManager.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(mm.getString(R.string.magisk_update_title))
                .setContentText(mm.getString(R.string.magisk_update_available, mm.remoteMagiskVersionString))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(mm, SplashActivity.class);
        intent.putExtra(MagiskManager.INTENT_SECTION, "magisk");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(mm);
        stackBuilder.addParentStack(SplashActivity.class);
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void showManagerUpdateNotification(MagiskManager mm) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, MagiskManager.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(mm.getString(R.string.manager_update_title))
                .setContentText(mm.getString(R.string.manager_download_install))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true);
        Intent intent = new Intent(mm, ManagerUpdate.class);
        intent.putExtra(MagiskManager.INTENT_LINK, mm.managerLink);
        intent.putExtra(MagiskManager.INTENT_VERSION, mm.remoteManagerVersionString);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
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

    public static void showMagiskInstallDialog(MagiskFragment fragment, boolean enc, boolean verity) {
        MagiskManager mm = getMagiskManager(fragment.getActivity());
        String filename = getLegalFilename("Magisk-v" + mm.remoteMagiskVersionString + ".zip");
        new AlertDialogBuilder(fragment.getActivity())
            .setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.magisk)))
            .setMessage(mm.getString(R.string.repo_install_msg, filename))
            .setCancelable(true)
            .setPositiveButton(R.string.install, (d, i) -> {
                List<String> options = new ArrayList<>();
                options.add(mm.getString(R.string.download_zip_only));
                options.add(mm.getString(R.string.patch_boot_file));
                if (Shell.rootAccess()) {
                    options.add(mm.getString(R.string.direct_install));
                }
                List<String> res = Shell.getShell(mm).su("echo $SLOT");
                if (isValidShellResponse(res)) {
                    options.add(mm.getString(R.string.install_second_slot));
                }
                char[] slot = isValidShellResponse(res) ? res.get(0).toCharArray() : null;
                new AlertDialog.Builder(fragment.getActivity())
                    .setTitle(R.string.select_method)
                    .setItems(
                        options.toArray(new String [0]),
                        (dialog, idx) -> {
                            String boot;
                            DownloadReceiver receiver = null;
                            switch (idx) {
                                case 1:
                                    if (mm.remoteMagiskVersionCode < 1400) {
                                        mm.toast(R.string.no_boot_file_patch_support, Toast.LENGTH_LONG);
                                        return;
                                    }
                                    mm.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                                    intent.setType("*/*");
                                    fragment.startActivityForResult(intent, SELECT_BOOT_IMG,
                                        (requestCode, resultCode, data) -> {
                                            if (requestCode == SELECT_BOOT_IMG
                                                    && resultCode == Activity.RESULT_OK && data != null) {
                                                dlAndReceive(
                                                    fragment.getActivity(),
                                                    new DownloadReceiver() {
                                                        @Override
                                                        public void onDownloadDone(Uri uri) {
                                                            Intent intent = new Intent(mm, FlashActivity.class);
                                                            intent.setData(uri)
                                                                .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                                                .putExtra(FlashActivity.SET_BOOT, data.getData())
                                                                .putExtra(FlashActivity.SET_ENC, enc)
                                                                .putExtra(FlashActivity.SET_VERITY, verity)
                                                                .putExtra(FlashActivity.SET_ACTION, FlashActivity.PATCH_BOOT);
                                                            mm.startActivity(intent);
                                                        }
                                                    },
                                                    mm.magiskLink,
                                                    filename
                                                );
                                            }
                                        });
                                    return;
                                case 0:
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            showUriSnack(fragment.getActivity(), uri);
                                        }
                                    };
                                    break;
                                case 2:
                                    boot = fragment.getSelectedBootImage();
                                    if (boot == null)
                                        return;
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri)
                                                .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                                .putExtra(FlashActivity.SET_BOOT, boot)
                                                .putExtra(FlashActivity.SET_ENC, enc)
                                                .putExtra(FlashActivity.SET_VERITY, verity)
                                                .putExtra(FlashActivity.SET_ACTION, FlashActivity.FLASH_MAGISK);
                                            mm.startActivity(intent);
                                        }
                                    };
                                    break;
                                case 3:
                                    assert (slot != null);
                                    // Choose the other slot
                                    if (slot[1] == 'a') slot[1] = 'b';
                                    else slot[1] = 'a';
                                    // Then find the boot image again
                                    List<String> ret = Shell.getShell(mm).su(
                                            "BOOTIMAGE=",
                                            "SLOT=" + String.valueOf(slot),
                                            "find_boot_image",
                                            "echo \"$BOOTIMAGE\""
                                    );
                                    boot = isValidShellResponse(ret) ? ret.get(ret.size() - 1) : null;
                                    Shell.getShell(mm).su_raw("mount_partitions");
                                    if (boot == null)
                                        return;
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri)
                                                    .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                                    .putExtra(FlashActivity.SET_BOOT, boot)
                                                    .putExtra(FlashActivity.SET_ENC, enc)
                                                    .putExtra(FlashActivity.SET_VERITY, verity)
                                                    .putExtra(FlashActivity.SET_ACTION, FlashActivity.FLASH_MAGISK);
                                            mm.startActivity(intent);
                                        }
                                    };
                                default:
                            }
                            Utils.dlAndReceive(
                                    fragment.getActivity(),
                                    receiver,
                                    mm.magiskLink,
                                    filename
                            );
                        }
                    ).show();
            })
            .setNeutralButton(R.string.release_notes, (d, i) -> {
                if (mm.releaseNoteLink != null) {
                    Intent openLink = new Intent(Intent.ACTION_VIEW, Uri.parse(mm.releaseNoteLink));
                    openLink.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mm.startActivity(openLink);
                }
            })
            .setNegativeButton(R.string.no_thanks, null)
            .show();
    }

    public static void showManagerInstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        new AlertDialogBuilder(activity)
                .setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.app_name)))
                .setMessage(mm.getString(R.string.repo_install_msg,
                        Utils.getLegalFilename("MagiskManager-v" +
                                mm.remoteManagerVersionString + ".apk")))
                .setCancelable(true)
                .setPositiveButton(R.string.install, (d, i) -> {
                    Intent intent = new Intent(mm, ManagerUpdate.class);
                    intent.putExtra(MagiskManager.INTENT_LINK, mm.managerLink);
                    intent.putExtra(MagiskManager.INTENT_VERSION, mm.remoteManagerVersionString);
                    mm.sendBroadcast(intent);
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }

    public static void showUninstallDialog(MagiskFragment fragment) {
        MagiskManager mm = Utils.getMagiskManager(fragment.getActivity());
        new AlertDialogBuilder(fragment.getActivity())
            .setTitle(R.string.uninstall_magisk_title)
            .setMessage(R.string.uninstall_magisk_msg)
            .setPositiveButton(R.string.complete_uninstall, (d, i) -> {
                try {
                    InputStream in = mm.getAssets().open(UNINSTALLER);
                    File uninstaller = new File(mm.getCacheDir(), UNINSTALLER);
                    FileOutputStream out = new FileOutputStream(uninstaller);
                    byte[] bytes = new byte[1024];
                    int read;
                    while ((read = in.read(bytes)) != -1) {
                        out.write(bytes, 0, read);
                    }
                    in.close();
                    out.close();
                    in = mm.getAssets().open(UTIL_FUNCTIONS);
                    File utils = new File(mm.getCacheDir(), UTIL_FUNCTIONS);
                    out = new FileOutputStream(utils);
                    while ((read = in.read(bytes)) != -1) {
                        out.write(bytes, 0, read);
                    }
                    in.close();
                    out.close();
                    Shell.getShell(mm).su(
                            "cat " + uninstaller + " > /cache/" + UNINSTALLER,
                            "cat " + utils + " > /data/magisk/" + UTIL_FUNCTIONS
                    );
                    mm.toast(R.string.uninstall_toast, Toast.LENGTH_LONG);
                    Shell.getShell(mm).su_raw(
                            "sleep 5",
                            "pm uninstall " + mm.getApplicationInfo().packageName
                    );
                } catch (IOException e) {
                    e.printStackTrace();
                }
            })
            .setNeutralButton(R.string.restore_stock_boot, (d, i) -> {
                String boot = fragment.getSelectedBootImage();
                if (boot == null) return;
                new RestoreStockBoot(mm, boot).exec();
            })
            .setNegativeButton(R.string.no_thanks, null)
            .show();
    }

    public static boolean useFDE(Context context) {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
                && context.getSystemService(DevicePolicyManager.class).getStorageEncryptionStatus()
                == DevicePolicyManager.ENCRYPTION_STATUS_ACTIVE_PER_USER;
    }

    public static Context getEncContext(Context context) {
        if (useFDE(context))
            return context.createDeviceProtectedStorageContext();
        else
            return context;
    }
}