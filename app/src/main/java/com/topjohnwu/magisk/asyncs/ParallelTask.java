package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.lang.ref.WeakReference;

public abstract class ParallelTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {

    private WeakReference<Activity> weakActivity;
    private WeakReference<MagiskManager> weakMagiskManager;

    private Runnable callback = null;

    public ParallelTask() {}

    public ParallelTask(Context context) {
        weakMagiskManager = new WeakReference<>(Utils.getMagiskManager(context));
    }

    public ParallelTask(Activity context) {
        this((Context) context);
        weakActivity = new WeakReference<>(context);
    }

    protected Activity getActivity() {
        return weakActivity.get();
    }

    protected MagiskManager getMagiskManager() {
        return weakMagiskManager.get();
    }

    protected Shell getShell() {
        MagiskManager magiskManager = getMagiskManager();
        return magiskManager == null ? null : Shell.getShell(magiskManager);
    }

    @SuppressWarnings("unchecked")
    public ParallelTask<Params, Progress, Result> exec(Params... params) {
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
        return this;
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
