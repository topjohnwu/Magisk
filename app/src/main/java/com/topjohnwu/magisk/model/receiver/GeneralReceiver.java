package com.topjohnwu.magisk.model.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.SuLogger;
import com.topjohnwu.magisk.view.Notifications;
import com.topjohnwu.magisk.view.Shortcuts;
import com.topjohnwu.superuser.Shell;

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
            case Intent.ACTION_REBOOT:
            case Intent.ACTION_BOOT_COMPLETED:
                action = intent.getStringExtra("action");
                if (action == null) {
                    // Actual boot completed event
                    Shell.su("mm_patch_dtbo").submit(result -> {
                        if (result.isSuccess())
                            Notifications.dtboPatched();
                    });
                    break;
                }
                switch (action) {
                    case SuRequestActivity.REQUEST:
                        Intent i = new Intent(app, ClassMap.get(SuRequestActivity.class))
                                .setAction(action)
                                .putExtra("socket", intent.getStringExtra("socket"))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                .addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                        app.startActivity(i);
                        break;
                    case SuRequestActivity.LOG:
                        SuLogger.handleLogs(intent);
                        break;
                    case SuRequestActivity.NOTIFY:
                        SuLogger.handleNotify(intent);
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
