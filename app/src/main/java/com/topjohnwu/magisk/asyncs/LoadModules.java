package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.module.ModuleHelper;

public class LoadModules extends SerialTask<Void, Void, Void> {

    public LoadModules(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        ModuleHelper.createModuleMap(magiskManager);
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.moduleLoadDone.trigger();
    }
}
