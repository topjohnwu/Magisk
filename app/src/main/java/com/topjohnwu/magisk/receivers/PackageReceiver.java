package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.superuser.Policy;
import com.topjohnwu.magisk.utils.Utils;

public class PackageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager magiskManager = Utils.getMagiskManager(context);
        String pkg = intent.getData().getEncodedSchemeSpecificPart();
        Policy policy = magiskManager.suDB.getPolicy(pkg);
        if (policy == null)
            return;

        magiskManager.initSUConfig();

        switch (intent.getAction()) {
            case Intent.ACTION_PACKAGE_ADDED:
                int uid = intent.getIntExtra(Intent.EXTRA_UID, -1);
                // Update the UID if available
                if (uid > 0) {
                    policy.uid = uid % 100000;
                }
                magiskManager.suDB.updatePolicy(policy);
                return;
            case Intent.ACTION_PACKAGE_REMOVED:
                boolean isUpdate = intent.getBooleanExtra(Intent.EXTRA_REPLACING, false);
                if (!isUpdate || magiskManager.suReauth) {
                    magiskManager.suDB.deletePolicy(policy);
                }
                break;
        }
    }
}
