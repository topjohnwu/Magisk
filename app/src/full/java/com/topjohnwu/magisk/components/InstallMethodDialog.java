package com.topjohnwu.magisk.components;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import androidx.appcompat.app.AlertDialog;

class InstallMethodDialog extends AlertDialog.Builder {

    InstallMethodDialog(BaseActivity activity, List<String> options) {
        super(activity);
        setTitle(R.string.select_method);
        setItems(options.toArray(new String [0]), (dialog, idx) -> {
            Intent intent;
            switch (idx) {
                case 1:
                    Utils.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
                    intent = new Intent(Intent.ACTION_GET_CONTENT).setType("*/*");
                    activity.runWithPermission(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, () ->
                        activity.startActivityForResult(intent, Const.ID.SELECT_BOOT,
                        (requestCode, resultCode, data) -> {
                            if (requestCode == Const.ID.SELECT_BOOT &&
                                    resultCode == Activity.RESULT_OK && data != null) {
                                Intent i = new Intent(activity, Data.classMap.get(FlashActivity.class))
                                        .putExtra(Const.Key.FLASH_SET_BOOT, data.getData())
                                        .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_BOOT);
                                activity.startActivity(i);
                            }
                        })
                    );

                    break;
                case 0:
                    String filename = Utils.fmt("Magisk-v%s(%d).zip",
                            Data.remoteMagiskVersionString, Data.remoteMagiskVersionCode);
                    Download.receive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            SnackbarMaker.showUri(activity, uri);
                        }
                    }, Data.magiskLink, filename);
                    break;
                case 2:
                    intent = new Intent(activity, Data.classMap.get(FlashActivity.class))
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK);
                    activity.startActivity(intent);
                    break;
                case 3:
                    new CustomAlertDialog(activity)
                        .setTitle(R.string.warning)
                        .setMessage(R.string.install_inactive_slot_msg)
                        .setCancelable(true)
                        .setPositiveButton(R.string.yes, (d, i) -> {
                            Intent it = new Intent(activity, Data.classMap.get(FlashActivity.class))
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_INACTIVE_SLOT);
                            activity.startActivity(it);
                        })
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
                    break;
                default:
            }
        });
    }
}
