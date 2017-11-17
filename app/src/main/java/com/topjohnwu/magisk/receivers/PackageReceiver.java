package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

public class PackageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager mm = Utils.getMagiskManager(context);

        String pkg = intent.getData().getEncodedSchemeSpecificPart();

        switch (intent.getAction()) {
            case Intent.ACTION_PACKAGE_REPLACED:
                // This will only work pre-O
                if (mm.suReauth) {
                    mm.suDB.deletePolicy(pkg);
                }
                break;
            case Intent.ACTION_PACKAGE_FULLY_REMOVED:
                mm.suDB.deletePolicy(pkg);
                Shell.su_raw("magiskhide --rm " + pkg);
                break;
        }
    }
}
