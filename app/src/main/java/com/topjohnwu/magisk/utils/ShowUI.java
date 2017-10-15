package com.topjohnwu.magisk.utils;

import android.app.Activity;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.support.v7.app.AlertDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskFragment;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.asyncs.RestoreStockBoot;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ManagerUpdate;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class ShowUI {
    private static final int SELECT_BOOT_IMG = 3;
    private static final String UNINSTALLER = "magisk_uninstaller.sh";
    private static final int MAGISK_UPDATE_NOTIFICATION_ID = 1;
    private static final int APK_UPDATE_NOTIFICATION_ID = 2;

    public static void showMagiskUpdateNotification() {
        MagiskManager mm = MagiskManager.get();
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

    public static void showManagerUpdateNotification() {
        MagiskManager mm = MagiskManager.get();
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

    public static void showMagiskInstallDialog(MagiskFragment fragment, boolean enc, boolean verity) {
        MagiskManager mm = Utils.getMagiskManager(fragment.getContext());
        String filename = Utils.getLegalFilename("Magisk-v" + mm.remoteMagiskVersionString + ".zip");
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
                List<String> res = Shell.su("echo $SLOT");
                if (Utils.isValidShellResponse(res)) {
                    options.add(mm.getString(R.string.install_second_slot));
                }
                char[] slot = Utils.isValidShellResponse(res) ? res.get(0).toCharArray() : null;
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
                                        MagiskManager.toast(R.string.no_boot_file_patch_support, Toast.LENGTH_LONG);
                                        return;
                                    }
                                    MagiskManager.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                                    intent.setType("*/*");
                                    fragment.startActivityForResult(intent, SELECT_BOOT_IMG,
                                        (requestCode, resultCode, data) -> {
                                            if (requestCode == SELECT_BOOT_IMG
                                                    && resultCode == Activity.RESULT_OK && data != null) {
                                                Utils.dlAndReceive(
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
                                            Utils.showUriSnack(fragment.getActivity(), uri);
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
                                    List<String> ret = Shell.su(
                                            "BOOTIMAGE=",
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
                    in = mm.getAssets().open(Utils.UTIL_FUNCTIONS);
                    File utils = new File(mm.getCacheDir(), Utils.UTIL_FUNCTIONS);
                    out = new FileOutputStream(utils);
                    while ((read = in.read(bytes)) != -1) {
                        out.write(bytes, 0, read);
                    }
                    in.close();
                    out.close();
                    Shell.su(
                            "cat " + uninstaller + " > /cache/" + UNINSTALLER,
                            "cat " + utils + " > /data/magisk/" + Utils.UTIL_FUNCTIONS
                    );
                    MagiskManager.toast(R.string.uninstall_toast, Toast.LENGTH_LONG);
                    Shell.su_raw(
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
                new RestoreStockBoot(boot).exec();
            })
            .setNegativeButton(R.string.no_thanks, null)
            .show();
    }
}
