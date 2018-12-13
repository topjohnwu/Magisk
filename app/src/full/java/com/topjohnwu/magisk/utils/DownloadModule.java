package com.topjohnwu.magisk.utils;

import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.container.Repo;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.ProgressNotification;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class DownloadModule {

    public static void exec(BaseActivity activity, Repo repo, boolean install) {
        activity.runWithExternalRW(() -> AsyncTask.THREAD_POOL_EXECUTOR.execute(
                () -> dlProcessInstall(repo, install)));
    }

    private static void dlProcessInstall(Repo repo, boolean install) {
        File output = new File(Const.EXTERNAL_PATH, repo.getDownloadFilename());
        ProgressNotification progress = new ProgressNotification(output.getName());
        try {
            InputStream in = Networking.get(repo.getZipUrl())
                    .setDownloadProgressListener(progress)
                    .execForInputStream().getResult();
            removeTopFolder(in, new BufferedOutputStream(new FileOutputStream(output)));
            if (install) {
                progress.dismiss();
                Intent intent = new Intent(App.self, ClassMap.get(FlashActivity.class));
                intent.setData(Uri.fromFile(output))
                        .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                        .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
                App.self.startActivity(intent);
            } else {
                progress.getNotification().setContentTitle(output.getName());
                progress.dlDone();
            }
        } catch (Exception e) {
            e.printStackTrace();
            progress.dlFail();
        }
    }

    private static void removeTopFolder(InputStream in, OutputStream out) throws IOException {
        try (ZipInputStream zin = new ZipInputStream(in);
             ZipOutputStream zout = new ZipOutputStream(out)) {
            ZipEntry entry;
            int off = -1;
            while ((entry = zin.getNextEntry()) != null) {
                if (off < 0)
                    off = entry.getName().indexOf('/') + 1;
                String path = entry.getName().substring(off);
                if (path.isEmpty())
                    continue;
                zout.putNextEntry(new ZipEntry(path));
                if (!entry.isDirectory())
                    ShellUtils.pump(zin, zout);
            }
        }
    }

}
