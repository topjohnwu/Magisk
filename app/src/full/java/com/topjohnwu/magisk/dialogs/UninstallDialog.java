package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.text.TextUtils;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.uicomponents.ProgressNotification;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.io.File;

public class UninstallDialog extends CustomAlertDialog {

    public UninstallDialog(@NonNull Activity activity) {
        super(activity);
        setTitle(R.string.uninstall_magisk_title);
        setMessage(R.string.uninstall_magisk_msg);
        setNeutralButton(R.string.restore_img, (d, i) -> {
            ProgressDialog dialog = ProgressDialog.show(activity,
                    activity.getString(R.string.restore_img),
                    activity.getString(R.string.restore_img_msg));
            Shell.su("restore_imgs").submit(result -> {
                dialog.cancel();
                if (result.isSuccess()) {
                    Utils.toast(R.string.restore_done, Toast.LENGTH_SHORT);
                } else {
                    Utils.toast(R.string.restore_fail, Toast.LENGTH_LONG);
                }
            });
        });
        if (!TextUtils.isEmpty(Config.uninstallerLink)) {
            setPositiveButton(R.string.complete_uninstall, (d, i) -> {
                File zip = new File(activity.getFilesDir(), "uninstaller.zip");
                ProgressNotification progress = new ProgressNotification(zip.getName());
                Networking.get(Config.uninstallerLink)
                    .setDownloadProgressListener(progress)
                    .setErrorHandler(((conn, e) -> progress.dlFail()))
                    .getAsFile(zip, f -> {
                        progress.dismiss();
                        Intent intent = new Intent(activity, ClassMap.get(FlashActivity.class))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                .setData(Uri.fromFile(f))
                                .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL);
                        activity.startActivity(intent);
                    });
            });
        }
    }
}
