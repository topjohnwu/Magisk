package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.utils.Shell;

public abstract class RootTask <Params, Progress, Result> extends ParallelTask<Params, Progress, Result> {

    public RootTask() {}

    public RootTask(Activity context) {
        super(context);
    }

    @SafeVarargs
    @Override
    final protected Result doInBackground(Params... params) {
        synchronized (Shell.lock) {
            return doInRoot(params);
        }
    }

    @SuppressWarnings("unchecked")
    abstract protected Result doInRoot(Params... params);

    @SuppressWarnings("unchecked")
    @Override
    public void exec(Params... params) {
        if (Shell.rootAccess()) {
            super.exec(params);
        }
    }
}
