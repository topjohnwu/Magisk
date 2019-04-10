package com.topjohnwu.magisk.components;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SuRequestActivity;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.uicomponents.Notifications;
import com.topjohnwu.magisk.uicomponents.Shortcuts;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.SuLogger;
import com.topjohnwu.superuser.Shell;

public class GeneralReceiver extends BroadcastReceiver {

    private static SuLogger SU_LOGGER = new SuLogger() {
        @Override
        public String getMessage(Policy policy) {
            return App.self.getString(policy.policy == Policy.ALLOW ?
                    R.string.su_allow_toast : R.string.su_deny_toast, policy.appName);
        }
    };

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
            case Intent.ACTION_REBOOT:
            case Intent.ACTION_BOOT_COMPLETED:
                action = intent.getStringExtra("action");
                if (action == null)
                    action = "boot_complete";
                switch (action) {
                    case "request":
                        Intent i = new Intent(app, ClassMap.get(SuRequestActivity.class))
                                .putExtra("socket", intent.getStringExtra("socket"))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        if (Build.VERSION.CODENAME.equals("Q")) {
                            RootUtils.startActivity(i);
                        } else {
                            app.startActivity(i);
                        }
                        break;
                    case "log":
                        SU_LOGGER.handleLogs(intent);
                        break;
                    case "notify":
                        SU_LOGGER.handleNotify(intent);
                        break;
                    case "boot_complete":
                    default:
                        /* Devices with DTBO might want to patch dtbo.img.
                         * However, that is not possible if Magisk is installed by
                         * patching boot image with Magisk Manager and flashed via
                         * fastboot, since at that time we do not have root.
                         * Check for dtbo status every boot time, and prompt user
                         * to reboot if dtbo wasn't patched and patched by Magisk Manager.
                         * */
                        Shell.su("mm_patch_dtbo").submit(result -> {
                            if (result.isSuccess())
                                Notifications.dtboPatched();
                        });
                        break;
                }
                break;
            case Intent.ACTION_PACKAGE_REPLACED:
                // This will only work pre-O
                if (Config.get(Config.Key.SU_REAUTH)) {
                    app.mDB.deletePolicy(getPkg(intent));
                }
                break;
            case Intent.ACTION_PACKAGE_FULLY_REMOVED:
                String pkg = getPkg(intent);
                app.mDB.deletePolicy(pkg);
                Shell.su("magiskhide --rm " + pkg).submit();
                break;
            case Intent.ACTION_LOCALE_CHANGED:
                Shortcuts.setup(context);
                break;
            case Const.Key.BROADCAST_MANAGER_UPDATE:
                Config.managerLink = intent.getStringExtra(Const.Key.INTENT_SET_LINK);
                DownloadApp.upgrade(intent.getStringExtra(Const.Key.INTENT_SET_NAME));
                break;
            case Const.Key.BROADCAST_REBOOT:
                Shell.su("/system/bin/reboot").submit();
                break;
        }
    }
}
