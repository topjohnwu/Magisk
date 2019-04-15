package com.topjohnwu.magisk.view.dialogs;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ui.base.IBaseLeanback;
import com.topjohnwu.magisk.ui.flash.FlashActivity;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.view.ProgressNotification;
import com.topjohnwu.magisk.view.SnackbarMaker;
import com.topjohnwu.net.Networking;

import java.io.File;
import java.util.List;

import androidx.appcompat.app.AlertDialog;

class InstallMethodDialog extends AlertDialog.Builder {

    <Ctxt extends Activity & IBaseLeanback> InstallMethodDialog(Ctxt activity, List<String> options) {
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

    private <Ctxt extends Activity & IBaseLeanback> void patchBoot(Ctxt activity) {
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

    private <Ctxt extends Activity & IBaseLeanback> void downloadOnly(Ctxt activity) {
        activity.runWithExternalRW(() -> {
            String filename = Utils.fmt("Magisk-v%s(%d).zip",
                    Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode);
            File zip = new File(Const.EXTERNAL_PATH, filename);
            ProgressNotification progress = new ProgressNotification(filename);
            Networking.get(Config.magiskLink)
                    .setDownloadProgressListener(progress)
                    .setErrorHandler((conn, e) -> progress.dlFail())
                    .getAsFile(zip, f -> {
                        progress.dlDone();
                        SnackbarMaker.make(activity,
                                activity.getString(R.string.internal_storage, "/Download/" + filename),
                                Snackbar.LENGTH_LONG).show();
                    });
        });
    }

    private <Ctxt extends Activity & IBaseLeanback> void installInactiveSlot(Ctxt activity) {
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
