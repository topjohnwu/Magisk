package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.net.Networking;

import java.io.File;
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

    private void patchBoot(BaseActivity a) {
        Utils.toast(R.string.boot_file_patch_msg, Toast.LENGTH_LONG);
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT).setType("*/*");
        a.runWithExternalRW(() ->
                a.startActivityForResult(intent, Const.ID.SELECT_BOOT,
                        (requestCode, resultCode, data) -> {
                            if (requestCode == Const.ID.SELECT_BOOT &&
                                    resultCode == Activity.RESULT_OK && data != null) {
                                Intent i = new Intent(a, ClassMap.get(FlashActivity.class))
                                        .setData(data.getData())
                                        .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_BOOT);
                                a.startActivity(i);
                            }
                        })
        );
    }

    private void downloadOnly(BaseActivity a) {
        a.runWithExternalRW(() -> {
            String filename = Utils.fmt("Magisk-v%s(%d).zip",
                    Data.remoteMagiskVersionString, Data.remoteMagiskVersionCode);
            File zip = new File(Const.EXTERNAL_PATH, filename);
            ProgressNotification progress = new ProgressNotification(filename);
            Networking.get(Data.magiskLink)
                    .setDownloadProgressListener(progress)
                    .setErrorHandler(((conn, e) -> progress.dlFail()))
                    .getAsFile(zip, f -> {
                        progress.dlDone();
                        SnackbarMaker.make(a,
                                a.getString(R.string.internal_storage, "/Download/" + filename),
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
