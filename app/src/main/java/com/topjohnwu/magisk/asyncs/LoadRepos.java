package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.module.ModuleHelper;

public class LoadRepos extends ParallelTask<Void, Void, Void> {

    public LoadRepos(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        ModuleHelper.createRepoMap(magiskManager);
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.repoLoadDone.trigger();
    }
}
