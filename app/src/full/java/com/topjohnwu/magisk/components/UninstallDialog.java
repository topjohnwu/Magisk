package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.text.TextUtils;
import android.widget.Toast;

import com.androidnetworking.AndroidNetworking;
import com.androidnetworking.error.ANError;
import com.androidnetworking.interfaces.DownloadListener;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.io.File;

import androidx.annotation.NonNull;

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
        if (!TextUtils.isEmpty(Data.uninstallerLink)) {
            setPositiveButton(R.string.complete_uninstall, (d, i) -> {
                File zip = new File(activity.getFilesDir(), "uninstaller.zip");
                ProgressNotification progress = new ProgressNotification(zip.getName());
                AndroidNetworking.download(Data.uninstallerLink, zip.getParent(), zip.getName())
                    .build()
                    .setDownloadProgressListener(progress)
                    .startDownload(new DownloadListener() {
                        @Override
                        public void onDownloadComplete() {
                            progress.dismiss();
                            Intent intent = new Intent(activity, Data.classMap.get(FlashActivity.class))
                                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(Uri.fromFile(zip))
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL);
                            activity.startActivity(intent);
                        }

                        @Override
                        public void onError(ANError anError) {
                            progress.dlFail();
                        }
                    });
            });
        }
    }
}
