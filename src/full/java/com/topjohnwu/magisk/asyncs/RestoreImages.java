package com.topjohnwu.magisk.asyncs;

import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

public class RestoreImages extends ParallelTask<Void, Void, Boolean> {

    @Override
    protected Boolean doInBackground(Void... voids) {
        String sha1;
        sha1 = RootUtils.cmd("cat /.backup/.sha1");
        if (sha1 == null) {
            sha1 = RootUtils.cmd("cat /init.magisk.rc | grep STOCKSHA1");
            if (sha1 == null)
                return false;
            sha1 = sha1.substring(sha1.indexOf('=') + 1);
        }

        return ShellUtils.fastCmdResult(Shell.getShell(), "restore_imgs " + sha1);
    }

    @Override
    protected void onPostExecute(Boolean result) {
        if (result) {
            MagiskManager.toast(R.string.restore_done, Toast.LENGTH_SHORT);
        } else {
            MagiskManager.toast(R.string.restore_fail, Toast.LENGTH_LONG);
        }
    }
}
