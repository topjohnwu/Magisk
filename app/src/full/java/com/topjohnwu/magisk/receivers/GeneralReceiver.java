package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.SuRequestActivity;
import com.topjohnwu.magisk.services.OnBootService;
import com.topjohnwu.magisk.utils.DlInstallManager;
import com.topjohnwu.magisk.utils.SuConnector;
import com.topjohnwu.superuser.Shell;

public class GeneralReceiver extends BroadcastReceiver {

    private String getPkg(Intent i) {
        return i.getData() == null ? "" : i.getData().getEncodedSchemeSpecificPart();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        MagiskManager mm = Data.MM();
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
                        Intent i = new Intent(mm, Data.classMap.get(SuRequestActivity.class))
                                .putExtra("socket", intent.getStringExtra("socket"))
                                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        mm.startActivity(i);
                        break;
                    case "log":
                        SuConnector.handleLogs(intent);
                        break;
                    case "notify":
                        SuConnector.handleNotify(intent);
                        break;
                    case "boot":
                    default:
                        /* The actual on-boot trigger */
                        OnBootService.enqueueWork(mm);
                        break;
                }
                break;
            case Intent.ACTION_PACKAGE_REPLACED:
                // This will only work pre-O
                if (mm.prefs.getBoolean(Const.Key.SU_REAUTH, false)) {
                    mm.mDB.deletePolicy(getPkg(intent));
                }
                break;
            case Intent.ACTION_PACKAGE_FULLY_REMOVED:
                String pkg = getPkg(intent);
                mm.mDB.deletePolicy(pkg);
                Shell.su("magiskhide --rm " + pkg).submit();
                break;
            case Const.Key.BROADCAST_MANAGER_UPDATE:
                Data.managerLink = intent.getStringExtra(Const.Key.INTENT_SET_LINK);
                DlInstallManager.upgrade(intent.getStringExtra(Const.Key.INTENT_SET_NAME));
                break;
            case Const.Key.BROADCAST_REBOOT:
                Shell.su("/system/bin/reboot").submit();
                break;
        }
    }
}
