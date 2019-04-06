package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.uicomponents.ProgressNotification;
import com.topjohnwu.magisk.uicomponents.SnackbarMaker;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;

import java.io.File;
import java.util.List;

class InstallMethodDialog extends AlertDialog.Builder {

    InstallMethodDialog(BaseActivity activity, List<String> options) {
        super(activity);
        setTitle(R.string.select_method);
        setItems(options.toArray(new String[0]), (dialog, idx) -> {
            Intent intent;
            switch (idx) {
                case 1:
                    patchBoot(activity);
                    break;
                case 0:
                    downloadOnly(activity);
                    break;
                case 2:
                    intent = new Intent(activity, ClassMap.get(FlashActivity.class))
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK);
                    activity.startActivity(intent);
                    break;
                case 3:
                    installInactiveSlot(activity);
                    break;
                default:
            }
        });
    }

    private void patchBoot(BaseActivity activity) {
        activity.runWithExternalRW(() -> {
            Utils.toast(R.string.patch_file_msg, Toast.LENGTH_LONG);
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT)
                    .setType("*/*")
                    .addCategory(Intent.CATEGORY_OPENABLE);
            activity.startActivityForResult(intent, Const.ID.SELECT_BOOT,
                    (resultCode, data) -> {
                        if (resultCode == Activity.RESULT_OK && data != null) {
                            Intent i = new Intent(activity, ClassMap.get(FlashActivity.class))
                                    .setData(data.getData())
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_FILE);
                            activity.startActivity(i);
                        }
                    });
        });
    }

    private void downloadOnly(BaseActivity activity) {
        activity.runWithExternalRW(() -> {
            String filename = Utils.fmt("Magisk-v%s(%d).zip",
                    Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode);
            File zip = new File(Const.EXTERNAL_PATH, filename);
            ProgressNotification progress = new ProgressNotification(filename);
            Networking.get(Config.magiskLink)
                    .setDownloadProgressListener(progress)
                    .setErrorHandler(((conn, e) -> progress.dlFail()))
                    .getAsFile(zip, f -> {
                        progress.dlDone();
                        SnackbarMaker.make(activity,
                                activity.getString(R.string.internal_storage, "/Download/" + filename),
                                Snackbar.LENGTH_LONG).show();
                    });
        });
    }

    private void installInactiveSlot(BaseActivity activity) {
        new CustomAlertDialog(activity)
                .setTitle(R.string.warning)
                .setMessage(R.string.install_inactive_slot_msg)
                .setCancelable(true)
                .setPositiveButton(R.string.yes, (d, i) -> {
                    Intent intent = new Intent(activity, ClassMap.get(FlashActivity.class))
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_INACTIVE_SLOT);
                    activity.startActivity(intent);
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }
}
