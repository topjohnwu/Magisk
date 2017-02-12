package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;

public class MagiskHide extends SerialTask<Object, Void, Void> {

    @Override
    protected Void doInBackground(Object... params) {
        String command = (String) params[0];
        Shell.su(MagiskManager.MAGISK_HIDE_PATH + command);
        return null;
    }

    public void add(CharSequence packageName) {
        exec("add " + packageName);
    }

    public void rm(CharSequence packageName) {
        exec("rm " + packageName);
    }

    public void enable() {
        exec("enable; setprop persist.magisk.hide 1");
    }

    public void disable() {
        exec("disable; setprop persist.magisk.hide 0");
    }

}
