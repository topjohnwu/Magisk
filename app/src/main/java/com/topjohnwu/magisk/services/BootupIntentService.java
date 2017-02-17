package com.topjohnwu.magisk.services;

import android.app.IntentService;
import android.content.Intent;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class BootupIntentService extends IntentService {

    public BootupIntentService() {
        super("BootupIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        MagiskManager magiskManager = Utils.getMagiskManager(this);
        magiskManager.initSuAccess();
        magiskManager.updateMagiskInfo();
        List<String> ret = Shell.sh("getprop persist.magisk.hide");
        boolean started = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        if (magiskManager.prefs.getBoolean("magiskhide", false) &&
                !magiskManager.disabled && !started && magiskManager.magiskVersion > 11) {
            magiskManager.toast(R.string.start_magiskhide, Toast.LENGTH_LONG);
            Shell.su(true, MagiskManager.MAGISK_HIDE_PATH + "enable",
                    "setprop persist.magisk.hide 1");
        }
    }
}
