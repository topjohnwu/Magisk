package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;

public abstract class ParallelTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {
    protected Activity activity;
    protected MagiskManager magiskManager;

    public ParallelTask() {}

    public ParallelTask(Activity context) {
        activity = context;
        magiskManager = Utils.getMagiskManager(context);
    }

    @SafeVarargs
    public final void exec(Params... params) {
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
    }
}
