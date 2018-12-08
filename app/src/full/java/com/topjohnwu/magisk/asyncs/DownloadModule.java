package com.topjohnwu.magisk.asyncs;

import android.Manifest;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.ProgressNotification;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class DownloadModule {

    public static void exec(BaseActivity activity, Repo repo, boolean install) {
        activity.runWithPermission(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE },
                () -> AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> dlProcessInstall(repo, install)));
    }

    private static void dlProcessInstall(Repo repo, boolean install) {
        File output = new File(Const.EXTERNAL_PATH, repo.getDownloadFilename());
        ProgressNotification progress = new ProgressNotification(output.getName());
        try {
            MagiskManager mm = Data.MM();
            HttpURLConnection conn = WebService.mustRequest(repo.getZipUrl());
            ProgressInputStream pis = new ProgressInputStream(conn.getInputStream(),
                    conn.getContentLength(), progress);
            removeTopFolder(new BufferedInputStream(pis),
                    new BufferedOutputStream(new FileOutputStream(output)));
            conn.disconnect();
            if (install) {
                progress.dismiss();
                Intent intent = new Intent(mm, Data.classMap.get(FlashActivity.class));
                intent.setData(Uri.fromFile(output))
                        .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                        .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
                mm.startActivity(intent);
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

    private static class ProgressInputStream extends FilterInputStream {

        private long totalBytes;
        private long bytesDownloaded;
        private ProgressNotification progress;

        protected ProgressInputStream(InputStream in, long size, ProgressNotification p) {
            super(in);
            totalBytes = size;
            progress = p;
        }

        private void updateProgress() {
            progress.onProgress(bytesDownloaded, totalBytes);
        }

        @Override
        public int read() throws IOException {
            int b = super.read();
            if (b >= 0) {
                bytesDownloaded++;
                updateProgress();
            }
            return b;
        }

        @Override
        public int read(byte[] b) throws IOException {
            return read(b, 0, b.length);
        }

        @Override
        public int read(byte[] b, int off, int len) throws IOException {
            int sz = super.read(b, off, len);
            if (sz > 0) {
                bytesDownloaded += sz;
                updateProgress();
            }
            return sz;
        }
    }
}
