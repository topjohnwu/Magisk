package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.TextUtils;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.RestoreImages;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;

public class UninstallDialog extends CustomAlertDialog {

    public UninstallDialog(@NonNull Activity activity) {
        super(activity);
        MagiskManager mm = Utils.getMagiskManager(activity);
        setTitle(R.string.uninstall_magisk_title);
        setMessage(R.string.uninstall_magisk_msg);
        setNeutralButton(R.string.restore_img, (d, i) -> new RestoreImages(activity).exec());
        if (!TextUtils.isEmpty(mm.uninstallerLink)) {
            setPositiveButton(R.string.complete_uninstall, (d, i) ->
                    Utils.dlAndReceive(activity, new DownloadReceiver() {
                        @Override
                        public void onDownloadDone(Context context, Uri uri) {
                            Intent intent = new Intent(context, FlashActivity.class)
                                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(uri)
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL);
                            context.startActivity(intent);
                        }
                    }, mm.uninstallerLink, "magisk-uninstaller.zip"));
        }
    }
}
