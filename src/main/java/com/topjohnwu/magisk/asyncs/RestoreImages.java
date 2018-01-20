package com.topjohnwu.magisk.asyncs;

import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.List;

public class RestoreImages extends ParallelTask<Void, Void, Boolean> {

    @Override
    protected Boolean doInBackground(Void... voids) {
        String sha1;
        List<String> ret = Utils.readFile("/.backup/.sha1");
        if (Utils.isValidShellResponse(ret)) {
            sha1 = ret.get(0);
        } else {
            ret = Shell.su("cat /init.magisk.rc | grep STOCKSHA1");
            if (!Utils.isValidShellResponse(ret))
                return false;
            sha1 = ret.get(0).substring(ret.get(0).indexOf('=') + 1);
        }

        ret = Shell.su("restore_imgs " + sha1 + " && echo true || echo false");

        return Utils.isValidShellResponse(ret) && Boolean.parseBoolean(ret.get(ret.size() - 1));
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
