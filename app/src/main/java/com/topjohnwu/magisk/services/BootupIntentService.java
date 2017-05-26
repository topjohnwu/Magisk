package com.topjohnwu.magisk.services;

import android.app.IntentService;
import android.content.Intent;

public class BootupIntentService extends IntentService {

    public BootupIntentService() {
        super("BootupIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        /* Currently no bootup task to do
         MagiskManager magiskManager = Utils.getMagiskManager(this);
         magiskManager.initSuAccess();
         magiskManager.updateMagiskInfo();
         */
    }
}
