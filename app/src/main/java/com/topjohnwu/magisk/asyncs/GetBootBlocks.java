package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.utils.Utils;

public class GetBootBlocks extends ParallelTask<Void, Void, Void> {

    public GetBootBlocks(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... params) {
        magiskManager.blockList = magiskManager.rootShell.su(
                "find /dev/block -type b -maxdepth 1 | grep -v -E \"loop|ram|dm-0\""
        );
        if (magiskManager.bootBlock == null) {
            magiskManager.bootBlock = Utils.detectBootImage(magiskManager.rootShell);
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.blockDetectionDone.trigger();
        super.onPostExecute(v);
    }
}
