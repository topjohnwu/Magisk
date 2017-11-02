package com.topjohnwu.magisk.asyncs;

import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class RestoreStockBoot extends ParallelTask<Void, Void, Boolean> {

    private String mBoot;

    public RestoreStockBoot(String boot) {
        mBoot = boot;
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        String sha1;
        List<String> ret = Utils.readFile("/.backup/.sha1");
        if (!Utils.isValidShellResponse(ret)) {
            ret = Shell.su("cat /init.magisk.rc | grep STOCKSHA1");
            if (!Utils.isValidShellResponse(ret))
                return false;
            sha1 = ret.get(0).substring(ret.get(0).indexOf('=') + 1);
        } else {
            sha1 = ret.get(0);
        }

        String stock_boot = "/data/stock_boot_" + sha1 + ".img.gz";
        if (!Utils.itemExist(stock_boot))
            return false;
        Shell.su_raw("flash_boot_image " + stock_boot + " " + mBoot);
        return true;
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
