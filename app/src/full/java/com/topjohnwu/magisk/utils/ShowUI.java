package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.support.v7.app.AlertDialog;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.InstallMagisk;
import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.asyncs.RestoreImages;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;
import com.topjohnwu.magisk.receivers.RebootReceiver;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.List;

public class ShowUI {

    public static void magiskUpdateNotification() {
        MagiskManager mm = MagiskManager.get();

        Intent intent = new Intent(mm, SplashActivity.class);
        intent.putExtra(Const.Key.OPEN_SECTION, "magisk");
        TaskStackBuilder stackBuilder = TaskStackBuilder.create(mm);
        stackBuilder.addParentStack(SplashActivity.class);
        stackBuilder.addNextIntent(intent);
        PendingIntent pendingIntent = stackBuilder.getPendingIntent(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(mm.getString(R.string.magisk_update_title))
                .setContentText(mm.getString(R.string.magisk_update_available, mm.remoteMagiskVersionString))
                .setVibrate(new long[]{0, 100, 100, 100})
                .setAutoCancel(true)
                .setContentIntent(pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build());
    }

    public static void managerUpdateNotification() {
        MagiskManager mm = MagiskManager.get();
        String filename = Utils.fmt("MagiskManager-v%s(%d).apk",
                mm.remoteManagerVersionString, mm.remoteManagerVersionCode);

        Intent intent = new Intent(mm, ManagerUpdate.class);
        intent.putExtra(Const.Key.INTENT_SET_LINK, mm.managerLink);
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

    public static void dtboPatchedNotification() {
        MagiskManager mm = MagiskManager.get();

        Intent intent = new Intent(mm, RebootReceiver.class);
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

    public static void envFixDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        String filename = Utils.fmt("Magisk-v%s(%d).zip",
                mm.remoteMagiskVersionString, mm.remoteMagiskVersionCode);
        new AlertDialogBuilder(activity)
                .setTitle(R.string.env_fix_title)
                .setMessage(R.string.env_fix_msg)
                .setCancelable(true)
                .setPositiveButton(R.string.yes, (d, i) -> {
                    Utils.dlAndReceive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            new InstallMagisk(activity, uri).exec();
                        }
                    }, mm.magiskLink, filename);
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }

    public static void magiskInstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        String filename = Utils.fmt("Magisk-v%s(%d).zip",
                mm.remoteMagiskVersionString, mm.remoteMagiskVersionCode);
        AlertDialog.Builder b = new AlertDialogBuilder(activity)
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
                new AlertDialog.Builder(activity)
                    .setTitle(R.string.select_method)
                    .setItems(
                        options.toArray(new String [0]),
                        (dialog, idx) -> {
                            DownloadReceiver receiver = null;
                            switch (idx) {
                                case 1:
                                    if (mm.remoteMagiskVersionCode < 1400) {
                                        MagiskManager.toast(R.string.no_boot_file_patch_support, Toast.LENGTH_LONG);
                                        return;
                                    }
                                    MagiskManager.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                                    intent.setType("*/*");
                                    ((com.topjohnwu.magisk.components.Activity) activity)
                                        .startActivityForResult(intent, Const.ID.SELECT_BOOT,
                                        (requestCode, resultCode, data) -> {
                                            if (requestCode == Const.ID.SELECT_BOOT
                                                    && resultCode == Activity.RESULT_OK && data != null) {
                                                Utils.dlAndReceive(
                                                    activity,
                                                    new DownloadReceiver() {
                                                        @Override
                                                        public void onDownloadDone(Context context, Uri uri) {
                                                            Intent intent = new Intent(mm, FlashActivity.class);
                                                            intent.setData(uri)
                                                                .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                                                .putExtra(Const.Key.FLASH_SET_BOOT, data.getData())
                                                                .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_BOOT);
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
                                        public void onDownloadDone(Context context, Uri uri) {
                                            SnackbarMaker.showUri(activity, uri);
                                        }
                                    };
                                    break;
                                case 2:
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Context context, Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri).putExtra(Const.Key.FLASH_ACTION,
                                                    Const.Value.FLASH_MAGISK);
                                            activity.startActivity(intent);
                                        }
                                    };
                                    break;
                                case 3:
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Context context, Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri).putExtra(Const.Key.FLASH_ACTION,
                                                    Const.Value.FLASH_SECOND_SLOT);
                                            activity.startActivity(intent);
                                        }
                                    };
                                default:
                            }
                            Utils.dlAndReceive(activity, receiver, mm.magiskLink, filename);
                        }
                    ).show();
            })
            .setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(mm.magiskNoteLink)) {
            b.setNeutralButton(R.string.release_notes, (d, i) -> {
                if (mm.magiskNoteLink.contains("forum.xda-developers")) {
                    // Open forum links in browser
                    Intent openLink = new Intent(Intent.ACTION_VIEW, Uri.parse(mm.magiskNoteLink));
                    openLink.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mm.startActivity(openLink);
                } else {
                    new MarkDownWindow(activity, null, mm.magiskNoteLink).exec();
                }
            });
        }
        b.show();
    }

    public static void managerInstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        String filename = Utils.fmt("MagiskManager-v%s(%d).apk",
                mm.remoteManagerVersionString, mm.remoteManagerVersionCode);
        AlertDialog.Builder b = new AlertDialogBuilder(activity)
            .setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.app_name)))
            .setMessage(mm.getString(R.string.repo_install_msg, filename))
            .setCancelable(true)
            .setPositiveButton(R.string.install, (d, i) -> {
                com.topjohnwu.magisk.components.Activity.runWithPermission(activity,
                        new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
                    Intent intent = new Intent(mm, ManagerUpdate.class);
                    intent.putExtra(Const.Key.INTENT_SET_LINK, mm.managerLink);
                    intent.putExtra(Const.Key.INTENT_SET_FILENAME, filename);
                    mm.sendBroadcast(intent);
                });
            })
            .setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(mm.managerNoteLink)) {
            b.setNeutralButton(R.string.app_changelog, (d, i) ->
                    new MarkDownWindow(activity, null, mm.managerNoteLink).exec());
        }
        b.show();
    }

    public static void uninstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        AlertDialog.Builder b = new AlertDialogBuilder(activity)
            .setTitle(R.string.uninstall_magisk_title)
            .setMessage(R.string.uninstall_magisk_msg)
            .setNeutralButton(R.string.restore_img, (d, i) -> new RestoreImages(activity).exec());
        if (!TextUtils.isEmpty(mm.uninstallerLink)) {
            b.setPositiveButton(R.string.complete_uninstall, (d, i) ->
                    Utils.dlAndReceive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            Intent intent = new Intent(context, FlashActivity.class)
                                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(uri)
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL);
                            context.startActivity(intent);
                        }
                    }, mm.uninstallerLink, "magisk-uninstaller.zip"));
        }
        b.show();
    }
}
