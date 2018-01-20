package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.support.v7.app.AlertDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.RestoreImages;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;
import com.topjohnwu.magisk.receivers.RebootReceiver;
import com.topjohnwu.superuser.Shell;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
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
        builder.setSmallIcon(R.drawable.ic_magisk)
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

        Intent intent = new Intent(mm, ManagerUpdate.class);
        intent.putExtra(Const.Key.INTENT_SET_LINK, mm.managerLink);
        intent.putExtra(Const.Key.INTENT_SET_VERSION, mm.remoteManagerVersionString);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mm,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(mm, Const.ID.NOTIFICATION_CHANNEL);
        builder.setSmallIcon(R.drawable.ic_magisk)
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
        builder.setSmallIcon(R.drawable.ic_magisk)
                .setContentTitle(mm.getString(R.string.dtbo_patched_title))
                .setContentText(mm.getString(R.string.dtbo_patched_reboot))
                .setVibrate(new long[]{0, 100, 100, 100})
                .addAction(R.drawable.ic_refresh, mm.getString(R.string.reboot), pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(Const.ID.DTBO_NOTIFICATION_ID, builder.build());
    }

    public static void magiskInstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        String filename = Utils.fmt("Magisk-v%s.zip", mm.remoteMagiskVersionString);
        new AlertDialogBuilder(activity)
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
                List<String> res = Shell.su("echo $SLOT");
                if (Utils.isValidShellResponse(res)) {
                    options.add(mm.getString(R.string.install_second_slot));
                }
                char[] slot = Utils.isValidShellResponse(res) ? res.get(0).toCharArray() : null;
                new AlertDialog.Builder(activity)
                    .setTitle(R.string.select_method)
                    .setItems(
                        options.toArray(new String [0]),
                        (dialog, idx) -> {
                            String boot;
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
                                                        public void onDownloadDone(Uri uri) {
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
                                        public void onDownloadDone(Uri uri) {
                                            Utils.showUriSnack(activity, uri);
                                        }
                                    };
                                    break;
                                case 2:
                                    boot = mm.bootBlock;
                                    if (boot == null)
                                        return;
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri)
                                                .putExtra(Const.Key.FLASH_SET_BOOT, boot)
                                                .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK);
                                            activity.startActivity(intent);
                                        }
                                    };
                                    break;
                                case 3:
                                    assert (slot != null);
                                    // Choose the other slot
                                    if (slot[1] == 'a') slot[1] = 'b';
                                    else slot[1] = 'a';
                                    // Then find the boot image again
                                    List<String> ret = Shell.su(
                                            "SLOT=" + String.valueOf(slot),
                                            "find_boot_image",
                                            "echo \"$BOOTIMAGE\""
                                    );
                                    boot = Utils.isValidShellResponse(ret) ? ret.get(ret.size() - 1) : null;
                                    Shell.su_raw("mount_partitions");
                                    if (boot == null)
                                        return;
                                    receiver = new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            Intent intent = new Intent(mm, FlashActivity.class);
                                            intent.setData(uri)
                                                    .putExtra(Const.Key.FLASH_SET_BOOT, boot)
                                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK);
                                            activity.startActivity(intent);
                                        }
                                    };
                                default:
                            }
                            Utils.dlAndReceive(
                                    activity,
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

    public static void managerInstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        new AlertDialogBuilder(activity)
            .setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.app_name)))
            .setMessage(mm.getString(R.string.repo_install_msg,
                    Utils.fmt("MagiskManager-v%s.apk", mm.remoteManagerVersionString)))
            .setCancelable(true)
            .setPositiveButton(R.string.install, (d, i) -> {
                Utils.runWithPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE, () -> {
                    Intent intent = new Intent(mm, ManagerUpdate.class);
                    intent.putExtra(Const.Key.INTENT_SET_LINK, mm.managerLink);
                    intent.putExtra(Const.Key.INTENT_SET_VERSION, mm.remoteManagerVersionString);
                    mm.sendBroadcast(intent);
                });
            })
            .setNegativeButton(R.string.no_thanks, null)
            .show();
    }

    public static void uninstallDialog(Activity activity) {
        MagiskManager mm = Utils.getMagiskManager(activity);
        new AlertDialogBuilder(activity)
            .setTitle(R.string.uninstall_magisk_title)
            .setMessage(R.string.uninstall_magisk_msg)
            .setPositiveButton(R.string.complete_uninstall, (d, i) -> {
                ByteArrayOutputStream uninstaller = new ByteArrayOutputStream();
                try (InputStream in = mm.getAssets().open(Const.UNINSTALLER)) {
                    Utils.inToOut(in, uninstaller);
                } catch (IOException e) {
                    e.printStackTrace();
                    return;
                }
                ByteArrayOutputStream utils = new ByteArrayOutputStream();
                try (InputStream in = mm.getAssets().open(Const.UTIL_FUNCTIONS)) {
                    Utils.inToOut(in, utils);
                } catch (IOException e) {
                    e.printStackTrace();
                    return;
                }

                Shell.su(
                        Utils.fmt("echo '%s' > /cache/%s", uninstaller.toString().replace("'", "'\\''"), Const.UNINSTALLER),
                        Utils.fmt("echo '%s' > %s/%s", utils.toString().replace("'", "'\\''"),
                                mm.magiskVersionCode >= 1464 ? "/data/adb/magisk" : "/data/magisk", Const.UTIL_FUNCTIONS)
                );
                try {
                    uninstaller.close();
                    utils.close();
                } catch (IOException ignored) {}

                MagiskManager.toast(R.string.uninstall_toast, Toast.LENGTH_LONG);
                new Handler().postDelayed(() -> Utils.uninstallPkg(mm.getPackageName()), 5000);
            })
            .setNeutralButton(R.string.restore_img, (d, i) -> new RestoreImages().exec())
            .setNegativeButton(R.string.uninstall_app, (d, i) -> Utils.uninstallPkg(mm.getPackageName()))
            .show();
    }
}
