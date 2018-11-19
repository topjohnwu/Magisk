package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import androidx.annotation.NonNull;

public class UninstallDialog extends CustomAlertDialog {

    public UninstallDialog(@NonNull Activity activity) {
        super(activity);
        MagiskManager mm = Data.MM();
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
            setPositiveButton(R.string.complete_uninstall, (d, i) ->
                    Download.receive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            Intent intent = new Intent(context, Data.classMap.get(FlashActivity.class))
                                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(uri)
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL);
                            context.startActivity(intent);
                        }
                    }, Data.uninstallerLink, "magisk-uninstaller.zip"));
        }
    }
}
