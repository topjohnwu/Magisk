package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.SuRequestActivity;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.SuConnector;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

public class GeneralReceiver extends BroadcastReceiver {

    private String getPkg(Intent i) {
        return i.getData() == null ? "" : i.getData().getEncodedSchemeSpecificPart();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        App app = App.self;
        if (intent == null)
            return;
        String action = intent.getAction();
        if (action == null)
            return;
        switch (action) {
            case Intent.ACTION_BOOT_COMPLETED:
                String bootAction = intent.getStringExtra("action");
                if (bootAction == null)
                    bootAction = "boot";
                switch (bootAction) {
                    case "request":
                        Intent i = new Intent(app, ClassMap.get(SuRequestActivity.class))
                                .putExtra("socket", intent.getStringExtra("socket"))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        app.startActivity(i);
                        break;
                    case "log":
                        SuConnector.handleLogs(intent);
                        break;
                    case "notify":
                        SuConnector.handleNotify(intent);
                        break;
                    case "boot":
                    default:
                        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                            /* Devices with DTBO might want to patch dtbo.img.
                             * However, that is not possible if Magisk is installed by
                             * patching boot image with Magisk Manager and flashed via
                             * fastboot, since at that time we do not have root.
                             * Check for dtbo status every boot time, and prompt user
                             * to reboot if dtbo wasn't patched and patched by Magisk Manager.
                             * */
                            if (Shell.rootAccess() && ShellUtils.fastCmdResult("mm_patch_dtbo"))
                                Notifications.dtboPatched();
                        });
                        break;
                }
                break;
            case Intent.ACTION_PACKAGE_REPLACED:
                // This will only work pre-O
                if (app.prefs.getBoolean(Const.Key.SU_REAUTH, false)) {
                    app.mDB.deletePolicy(getPkg(intent));
                }
                break;
            case Intent.ACTION_PACKAGE_FULLY_REMOVED:
                String pkg = getPkg(intent);
                app.mDB.deletePolicy(pkg);
                Shell.su("magiskhide --rm " + pkg).submit();
                break;
            case Const.Key.BROADCAST_MANAGER_UPDATE:
                Data.managerLink = intent.getStringExtra(Const.Key.INTENT_SET_LINK);
                DownloadApp.upgrade(intent.getStringExtra(Const.Key.INTENT_SET_NAME));
                break;
            case Const.Key.BROADCAST_REBOOT:
                Shell.su("/system/bin/reboot").submit();
                break;
        }
    }
}
