package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AlertDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

class InstallMethodDialog extends AlertDialog.Builder {

    InstallMethodDialog(Activity activity, List<String> options, String filename) {
        super(activity);
        MagiskManager mm = Utils.getMagiskManager(activity);
        setTitle(R.string.select_method);
        setItems(options.toArray(new String [0]), (dialog, idx) -> {
            DownloadReceiver receiver = null;
            switch (idx) {
                case 1:
                    if (mm.remoteMagiskVersionCode < 1400) {
                        SnackbarMaker.make(activity, R.string.no_boot_file_patch_support,
                                Snackbar.LENGTH_LONG).show();
                        return;
                    }
                    MagiskManager.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                    intent.setType("*/*");
                    activity.startActivityForResult(intent, Const.ID.SELECT_BOOT,
                            (requestCode, resultCode, data) -> {
                                if (requestCode == Const.ID.SELECT_BOOT &&
                                        resultCode == Activity.RESULT_OK && data != null) {
                                    Utils.dlAndReceive(activity, new SelectBoot(data),
                                            mm.magiskLink, filename);
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
                            Intent intent = new Intent(context, FlashActivity.class);
                            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(uri).putExtra(Const.Key.FLASH_ACTION,
                                    Const.Value.FLASH_MAGISK);
                            context.startActivity(intent);
                        }
                    };
                    break;
                case 3:
                    receiver = new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            Intent intent = new Intent(context, FlashActivity.class);
                            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(uri).putExtra(Const.Key.FLASH_ACTION,
                                    Const.Value.FLASH_SECOND_SLOT);
                            context.startActivity(intent);
                        }
                    };
                default:
            }
            Utils.dlAndReceive(activity, receiver, mm.magiskLink, filename);
        });
    }

    private class SelectBoot extends DownloadReceiver {

        private Intent data;

        public SelectBoot(Intent data) {
            this.data = data;
        }

        @Override
        public void onDownloadDone(Context context, Uri uri) {
            Intent intent = new Intent(context, FlashActivity.class);
            intent.setData(uri)
                    .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    .putExtra(Const.Key.FLASH_SET_BOOT, data.getData())
                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_BOOT);
            context.startActivity(intent);
        }
    }
}
