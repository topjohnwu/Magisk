package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.superuser.Policy;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

public class PackageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        SuDatabaseHelper suDB = new SuDatabaseHelper(context);
        String pkg = intent.getData().getEncodedSchemeSpecificPart();
        Policy policy = suDB.getPolicy(pkg);
        if (policy == null)
            return;

        MagiskManager magiskManager = Utils.getMagiskManager(context);
        magiskManager.initSUConfig();

        switch (intent.getAction()) {
            case Intent.ACTION_PACKAGE_ADDED:
                int uid = intent.getIntExtra(Intent.EXTRA_UID, -1);
                // Update the UID if available
                if (uid > 0) {
                    policy.uid = uid % 100000;
                }
                suDB.updatePolicy(policy);
                return;
            case Intent.ACTION_PACKAGE_REMOVED:
                boolean isUpdate = intent.getBooleanExtra(Intent.EXTRA_REPLACING, false);
                if (!isUpdate || magiskManager.suReauth) {
                    suDB.deletePolicy(policy);
                }
                break;
        }
    }
}
