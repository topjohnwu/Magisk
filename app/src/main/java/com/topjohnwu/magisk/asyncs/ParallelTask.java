package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;

public abstract class ParallelTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {

    protected Activity activity;
    protected MagiskManager magiskManager;

    private Runnable callback = null;

    public ParallelTask() {}

    public ParallelTask(Activity context) {
        activity = context;
        magiskManager = Utils.getMagiskManager(context);
    }

    @SuppressWarnings("unchecked")
    public void exec(Params... params) {
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
    }

    @Override
    protected void onPostExecute(Result result) {
        if (callback != null) callback.run();
    }

    public ParallelTask<Params, Progress, Result> setCallBack(Runnable next) {
        callback = next;
        return this;
    }
}
