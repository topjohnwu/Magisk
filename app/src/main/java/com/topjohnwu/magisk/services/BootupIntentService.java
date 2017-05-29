package com.topjohnwu.magisk.services;

import android.app.IntentService;
import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;

public class BootupIntentService extends IntentService {

    public BootupIntentService() {
        super("BootupIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        ((MagiskManager) getApplication()).initSU();
    }
}
