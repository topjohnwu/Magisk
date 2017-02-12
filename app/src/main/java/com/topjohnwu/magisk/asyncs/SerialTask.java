package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

/**
 * This class is only used for running root commands
 **/

public abstract class SerialTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {
    protected Activity activity;
    protected MagiskManager magiskManager;

    public SerialTask() {}

    public SerialTask(Activity context) {
        activity = context;
        magiskManager = Utils.getMagiskManager(context);
    }

    @SafeVarargs
    public final void exec(Params... params) {
        if (!Shell.rootAccess()) return;
        executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, params);
    }
}
