package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.utils.DlInstallManager;

public class ManagerUpdate extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Data.managerLink = intent.getStringExtra(Const.Key.INTENT_SET_LINK);
        DlInstallManager.upgrade(intent.getStringExtra(Const.Key.INTENT_SET_NAME));
    }
}
