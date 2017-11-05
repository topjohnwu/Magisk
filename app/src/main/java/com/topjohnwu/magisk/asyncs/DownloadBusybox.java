package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.os.Build;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.HttpURLConnection;

public class DownloadBusybox extends ParallelTask<Void, Void, Void> {

    @Override
    protected Void doInBackground(Void... voids) {
        Context context = MagiskManager.get();
        File busybox = new File(context.getCacheDir(), "busybox");
        Utils.removeItem(context.getApplicationInfo().dataDir + "/busybox");
        try (FileOutputStream out  = new FileOutputStream(busybox)) {

            HttpURLConnection conn = WebService.request(
                    Build.SUPPORTED_32_BIT_ABIS[0].contains("x86") ?
                            Const.Url.BUSYBOX_X86 :
                            Const.Url.BUSYBOX_ARM,
                    null
            );
            if (conn == null) throw new IOException();
            BufferedInputStream bis = new BufferedInputStream(conn.getInputStream());
            Utils.inToOut(bis, out);
            conn.disconnect();
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (busybox.exists()) {
            Shell.su(
                    "rm -rf " + Const.BUSYBOXPATH,
                    "mkdir -p " + Const.BUSYBOXPATH,
                    "cp " + busybox + " " + Const.BUSYBOXPATH,
                    "chmod -R 755 " + Const.BUSYBOXPATH,
                    Const.BUSYBOXPATH + "/busybox --install -s " + Const.BUSYBOXPATH
            );
            busybox.delete();
        }
        return null;
    }
}
