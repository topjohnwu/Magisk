package com.topjohnwu.magisk.asyncs;

import android.content.Context;

import com.topjohnwu.magisk.MagiskManager;

import java.util.List;

public class MagiskHide extends ParallelTask<Object, Void, Void> {

    private boolean isList = false;

    public MagiskHide(Context context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Object... params) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return null;
        String command = (String) params[0];
        List<String> ret = magiskManager.shell.su("magiskhide --" + command);
        if (isList) {
            magiskManager.magiskHideList = ret;
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return;
        if (isList) {
            magiskManager.magiskHideDone.trigger();
        }
        super.onPostExecute(v);
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
        if (getMagiskManager() == null) return;
        exec("ls");
    }

}
