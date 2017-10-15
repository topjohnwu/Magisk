package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class RestoreStockBoot extends ParallelTask<Void, Void, Boolean> {

    private String mBoot;

    public RestoreStockBoot(Context context, String boot) {
        super(context);
        mBoot = boot;
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        List<String> ret = Shell.su("cat /init.magisk.rc | grep STOCKSHA1");
        if (!Utils.isValidShellResponse(ret))
            return false;
        String stock_boot = "/data/stock_boot_" + ret.get(0).substring(ret.get(0).indexOf('=') + 1) + ".img.gz";
        if (!Utils.itemExist(stock_boot))
            return false;
        Shell.su_raw("flash_boot_image " + stock_boot + " " + mBoot);
        return true;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        if (result) {
            getMagiskManager().toast(R.string.restore_done, Toast.LENGTH_SHORT);
        } else {
            getMagiskManager().toast(R.string.restore_fail, Toast.LENGTH_LONG);
        }
    }
}
