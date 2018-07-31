package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AlertDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

class InstallMethodDialog extends AlertDialog.Builder {

    InstallMethodDialog(Activity activity, List<String> options, String filename) {
        super(activity);
        MagiskManager mm = Utils.getMagiskManager(activity);
        setTitle(R.string.select_method);
        setItems(options.toArray(new String [0]), (dialog, idx) -> {
            Intent intent;
            switch (idx) {
                case 1:
                    if (Data.remoteMagiskVersionCode < 1400) {
                        SnackbarMaker.make(activity, R.string.no_boot_file_patch_support,
                                Snackbar.LENGTH_LONG).show();
                        return;
                    }
                    Utils.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                    intent = new Intent(Intent.ACTION_GET_CONTENT).setType("*/*");
                    activity.startActivityForResult(intent, Const.ID.SELECT_BOOT,
                            (requestCode, resultCode, data) -> {
                                if (requestCode == Const.ID.SELECT_BOOT &&
                                        resultCode == Activity.RESULT_OK && data != null) {
                                    Intent i = new Intent(activity, FlashActivity.class)
                                            .putExtra(Const.Key.FLASH_SET_BOOT, data.getData())
                                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_BOOT);
                                    activity.startActivity(i);
                                }
                            });
                    break;
                case 0:
                    Download.receive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            SnackbarMaker.showUri(activity, uri);
                        }
                    }, Data.magiskLink, filename);
                    break;
                case 2:
                    intent = new Intent(activity, FlashActivity.class)
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK);
                    activity.startActivity(intent);
                    break;
                case 3:
                    intent = new Intent(activity, FlashActivity.class)
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_INACTIVE_SLOT);
                    activity.startActivity(intent);
                    break;
                default:
            }
        });
    }
}
