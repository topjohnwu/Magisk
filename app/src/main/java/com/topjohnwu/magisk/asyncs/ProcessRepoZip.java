package com.topjohnwu.magisk.asyncs;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.InputStreamWrapper;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;
import java.util.jar.JarOutputStream;

public class ProcessRepoZip extends ParallelTask<Void, Object, Boolean> {

    private ProgressDialog progressDialog;
    private boolean mInstall;
    private String mLink;
    private File mFile;
    private int progress = 0, total = -1;
    private Handler mHandler;

    public ProcessRepoZip(Activity context, String link, String filename, boolean install) {
        super(context);
        mLink = link;
        mFile = new File(Const.EXTERNAL_PATH, filename);
        mInstall = install;
        mHandler = new Handler();
    }

    private void removeTopFolder(InputStream in, File output) throws IOException {
        JarInputStream source = new JarInputStream(in);
        JarOutputStream dest = new JarOutputStream(new BufferedOutputStream(new FileOutputStream(output)));
        JarEntry entry;
        String path;
        while ((entry = source.getNextJarEntry()) != null) {
            // Remove the top directory from the path
            path = entry.getName().substring(entry.getName().indexOf("/") + 1);
            // If it's the top folder, ignore it
            if (path.isEmpty()) {
                continue;
            }
            // Don't include placeholder
            if (path.equals("system/placeholder")) {
                continue;
            }
            dest.putNextEntry(new JarEntry(path));
            Utils.inToOut(source, dest);
        }
        source.close();
        dest.close();
        in.close();
    }

    @Override
    protected void onPreExecute() {
        Activity activity = getActivity();
        mFile.getParentFile().mkdirs();
        progressDialog = ProgressDialog.show(activity, activity.getString(R.string.zip_download_title), activity.getString(R.string.zip_download_msg, 0));
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        Activity activity = getActivity();
        if (activity == null) return null;
        try {
            // Request zip from Internet
            HttpURLConnection conn;
            do {
                conn = WebService.request(mLink, null);
                if (conn == null) return null;
                total = conn.getContentLength();
                if (total < 0)
                    conn.disconnect();
                else
                    break;
            } while (true);

            InputStream in = new BufferedInputStream(new ProgressInputStream(conn.getInputStream()));

            // Temp files
            File temp1 = new File(activity.getCacheDir(), "1.zip");
            File temp2 = new File(temp1.getParentFile(), "2.zip");
            temp1.getParentFile().mkdir();

            // First remove top folder in Github source zip, Web -> temp1
            removeTopFolder(in, temp1);

            conn.disconnect();
            mHandler.post(() -> {
                progressDialog.setTitle(R.string.zip_process_title);
                progressDialog.setMessage(getActivity().getString(R.string.zip_process_msg));
            });

            // Then sign the zip for the first time, temp1 -> temp2
            ZipUtils.signZip(temp1, temp2, false);

            // Adjust the zip to prevent unzip issues, temp2 -> temp1
            ZipUtils.zipAdjust(temp2.getPath(), temp1.getPath());

            // Finally, sign the whole zip file again, temp1 -> temp2
            ZipUtils.signZip(temp1, temp2, true);

            // Write it to the target zip, temp2 -> file
            try (
                OutputStream out = new BufferedOutputStream(new FileOutputStream(mFile));
                InputStream source = new BufferedInputStream(new FileInputStream(temp2))
            ) {
                byte[] buffer = new byte[4096];
                int length;
                while ((length = source.read(buffer)) > 0)
                    out.write(buffer, 0, length);
            }

            // Delete temp files
            temp1.delete();
            temp2.delete();

            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    @Override
    protected void onPostExecute(Boolean result) {
        Activity activity = getActivity();
        if (activity == null) return;
        progressDialog.dismiss();
        Uri uri = Uri.fromFile(mFile);
        if (result) {
            if (Shell.rootAccess() && mInstall) {
                Intent intent = new Intent(activity, FlashActivity.class);
                intent.setData(uri).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
                activity.startActivity(intent);
            } else {
                Utils.showUriSnack(activity, uri);
            }
        } else {
            MagiskManager.toast(R.string.process_error, Toast.LENGTH_LONG);
        }
        super.onPostExecute(result);
    }

    @Override
    public ParallelTask<Void, Object, Boolean> exec(Void... voids) {
        Utils.runWithPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE,
                () -> super.exec(voids));
        return this;
    }

    private class ProgressInputStream extends InputStreamWrapper {

        ProgressInputStream(InputStream in) {
            super(in);
        }

        private void updateDlProgress(int step) {
            progress += step;
            progressDialog.setMessage(getActivity().getString(R.string.zip_download_msg, 100 * progress / total));
        }

        @Override
        public synchronized int read() throws IOException {
            int b = super.read();
            if (b > 0) {
                mHandler.post(() -> updateDlProgress(1));
            }
            return b;
        }

        @Override
        public int read(@NonNull byte[] b) throws IOException {
            return read(b, 0, b.length);
        }

        @Override
        public synchronized int read(@NonNull byte[] b, int off, int len) throws IOException {
            int read = super.read(b, off, len);
            if (read > 0) {
                mHandler.post(() -> updateDlProgress(read));
            }
            return read;
        }
    }
}
