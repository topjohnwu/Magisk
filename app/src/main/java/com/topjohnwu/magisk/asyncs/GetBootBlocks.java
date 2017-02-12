package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

public class GetBootBlocks extends SerialTask<Void, Void, Void> {

    public GetBootBlocks(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... params) {
        if (Shell.rootAccess()) {
            magiskManager.blockList = Shell.su("ls /dev/block | grep mmc");
            if (magiskManager.bootBlock == null)
                magiskManager.bootBlock = Utils.detectBootImage();
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.blockDetectionDone.trigger();
    }
}
