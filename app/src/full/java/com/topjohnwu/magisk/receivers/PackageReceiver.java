package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.superuser.Shell;

public class PackageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager mm = Data.MM();

        String pkg = intent.getData().getEncodedSchemeSpecificPart();

        switch (intent.getAction()) {
            case Intent.ACTION_PACKAGE_REPLACED:
                // This will only work pre-O
                if (mm.prefs.getBoolean(Const.Key.SU_REAUTH, false)) {
                    mm.mDB.deletePolicy(pkg);
                }
                break;
            case Intent.ACTION_PACKAGE_FULLY_REMOVED:
                mm.mDB.deletePolicy(pkg);
                Shell.su("magiskhide --rm " + pkg).submit();
                break;
        }
    }
}
