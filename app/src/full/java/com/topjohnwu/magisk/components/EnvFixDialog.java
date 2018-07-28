package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.support.annotation.NonNull;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.InstallMagisk;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Utils;

public class EnvFixDialog extends CustomAlertDialog {

    public EnvFixDialog(@NonNull Activity activity) {
        super(activity);
        MagiskManager mm = Utils.getMagiskManager(activity);
        String filename = Utils.fmt("Magisk-v%s(%d).zip",
                mm.remoteMagiskVersionString, mm.remoteMagiskVersionCode);
        setTitle(R.string.env_fix_title);
        setMessage(R.string.env_fix_msg);
        setCancelable(true);
        setPositiveButton(R.string.yes, (d, i) -> Utils.dlAndReceive(activity,
                new DownloadReceiver() {
                    @Override
                    public void onDownloadDone(Context context, Uri uri) {
                        new InstallMagisk(activity, uri).exec();
                    }
                }, mm.magiskLink, filename));
        setNegativeButton(R.string.no_thanks, null);
    }
}
