package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.os.AsyncTask;

import java.lang.ref.WeakReference;

public abstract class ParallelTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {

    private WeakReference<Activity> weakActivity;

    public ParallelTask() {}

    public ParallelTask(Activity context) {
        weakActivity = new WeakReference<>(context);
    }

    protected Activity getActivity() {
        return weakActivity.get();
    }

    @SuppressWarnings("unchecked")
    public void exec(Params... params) {
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
    }
}
