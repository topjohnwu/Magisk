package com.topjohnwu.magisk.superuser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.utils.SuConnector;

public class SuReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent != null)
            SuConnector.handleLogs(intent, 1);
    }
}
