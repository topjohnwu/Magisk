package com.topjohnwu.magisk.receivers;

import android.app.IntentService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        context.startService(new Intent(context, BootupIntentService.class));
    }

    public static class BootupIntentService extends IntentService {

        public BootupIntentService() {
            super("BootupIntentService");
        }

        @Override
        protected void onHandleIntent(Intent intent) {
            MagiskManager magiskManager = (MagiskManager) getApplicationContext();
            magiskManager.initSuAccess();
            magiskManager.updateMagiskInfo();
            if (magiskManager.prefs.getBoolean("magiskhide", false) &&
                    !magiskManager.disabled && magiskManager.magiskVersion > 10.3) {
                magiskManager.toast(R.string.start_magiskhide, Toast.LENGTH_LONG);
                Shell.su(true, Async.MAGISK_HIDE_PATH + "enable",
                        "touch " + MagiskManager.MAGISK_MANAGER_BOOT);
            }
        }
    }
}
