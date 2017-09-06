package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.os.Build;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class DownloadBusybox extends ParallelTask<Void, Void, Void> {

    private static final String BUSYBOX_ARM = "https://github.com/topjohnwu/ndk-busybox/releases/download/1.27.2/busybox-arm";
    private static final String BUSYBOX_X86 = "https://github.com/topjohnwu/ndk-busybox/releases/download/1.27.2/busybox-x86";

    private File busybox;

    public DownloadBusybox(Context context) {
        super(context);
        busybox = new File(context.getCacheDir(), "busybox");
    }

    @Override
    protected Void doInBackground(Void... voids) {
        Context context = getMagiskManager();
        Utils.removeItem(getShell(), context.getApplicationInfo().dataDir + "/busybox");
        try {
            FileOutputStream out  = new FileOutputStream(busybox);
            InputStream in = WebService.request(WebService.GET,
                    Build.SUPPORTED_32_BIT_ABIS[0].contains("x86") ?
                            BUSYBOX_X86 :
                            BUSYBOX_ARM,
                    null
            );
            if (in == null) throw new IOException();
            BufferedInputStream bis = new BufferedInputStream(in);
            byte[] buffer = new byte[4096];
            int len;
            while ((len = bis.read(buffer)) != -1) {
                out.write(buffer, 0, len);
            }
            out.close();
            bis.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
        if (busybox.exists()) {
            getShell().su_raw(
                    "rm -rf " + MagiskManager.BUSYBOXPATH,
                    "mkdir -p " + MagiskManager.BUSYBOXPATH,
                    "cp " + busybox + " " + MagiskManager.BUSYBOXPATH,
                    "chmod -R 755 " + MagiskManager.BUSYBOXPATH,
                    MagiskManager.BUSYBOXPATH + "/busybox --install -s " + MagiskManager.BUSYBOXPATH
            );
            busybox.delete();
        }
        return null;
    }
}
