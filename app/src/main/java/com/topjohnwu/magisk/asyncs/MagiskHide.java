package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.utils.Shell;

import java.util.List;

public class MagiskHide extends SerialTask<Object, Void, Void> {

    private boolean isList = false;

    public MagiskHide() {}

    public MagiskHide(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Object... params) {
        String command = (String) params[0];
        List<String> ret = Shell.su("magiskhide --" + command);
        if (isList) {
            magiskManager.magiskHideList = ret;
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        if (isList) {
            magiskManager.magiskHideDone.trigger();
        }
    }

    public void add(CharSequence packageName) {
        exec("add " + packageName);
    }

    public void rm(CharSequence packageName) {
        exec("rm " + packageName);
    }

    public void enable() {
        exec("enable");
    }

    public void disable() {
        exec("disable");
    }

    public void list() {
        isList = true;
        if (magiskManager == null) return;
        exec("ls");
    }

}
