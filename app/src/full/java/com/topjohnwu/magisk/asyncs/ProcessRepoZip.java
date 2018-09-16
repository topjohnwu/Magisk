package com.topjohnwu.magisk.asyncs;

import android.Manifest;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.magisk.utils.ZipUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;
import java.util.jar.JarOutputStream;

import androidx.annotation.NonNull;

public class ProcessRepoZip extends ParallelTask<Void, Object, Boolean> {

    private ProgressDialog progressDialog;
    private boolean mInstall;
    private File mFile;
    private Repo mRepo;
    private int progress = 0, total = -1;

    public ProcessRepoZip(BaseActivity context, Repo repo, boolean install) {
        super(context);
        mRepo = repo;
        mInstall = install && Shell.rootAccess();
        mFile = new File(Download.EXTERNAL_PATH, repo.getDownloadFilename());
    }

    private void removeTopFolder(File input, File output) throws IOException {
        JarEntry entry;
        try (
            JarInputStream in = new JarInputStream(new BufferedInputStream(new FileInputStream(input)));
            JarOutputStream out = new JarOutputStream(new BufferedOutputStream(new FileOutputStream(output)))
        ) {
            String path;
            while ((entry = in.getNextJarEntry()) != null) {
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
                out.putNextEntry(new JarEntry(path));
                ShellUtils.pump(in, out);
            }
        }
    }

    @Override
    protected BaseActivity getActivity() {
        return (BaseActivity) super.getActivity();
    }

    @Override
    protected void onPreExecute() {
        BaseActivity activity = getActivity();
        mFile.getParentFile().mkdirs();
        progressDialog = ProgressDialog.show(activity, activity.getString(R.string.zip_download_title), activity.getString(R.string.zip_download_msg, 0));
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        BaseActivity activity = getActivity();
        if (activity == null) return null;
        try {
            // Request zip from Internet
            HttpURLConnection conn = WebService.mustRequest(mRepo.getZipUrl(), null);
            total = conn.getContentLength();

            // Temp files
            File temp1 = new File(activity.getCacheDir(), "1.zip");
            File temp2 = new File(temp1.getParentFile(), "2.zip");
            temp1.getParentFile().mkdir();

            // First download the zip, Web -> temp1
            try (
                InputStream in = new BufferedInputStream(new ProgressInputStream(conn.getInputStream()));
                OutputStream out = new BufferedOutputStream(new FileOutputStream(temp1))
            ) {
                ShellUtils.pump(in, out);
                in.close();
            }
            conn.disconnect();

            Data.mainHandler.post(() -> {
                progressDialog.setTitle(R.string.zip_process_title);
                progressDialog.setMessage(getActivity().getString(R.string.zip_process_msg));
            });

            // First remove top folder in Github source zip, temp1 -> temp2
            removeTopFolder(temp1, temp2);

            // Then sign the zip
            ZipUtils.signZip(temp2, mFile);

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
        BaseActivity activity = getActivity();
        if (activity == null) return;
        progressDialog.dismiss();
        if (result) {
            Uri uri = Uri.fromFile(mFile);
            if (mInstall) {
                Intent intent = new Intent(activity, Data.classMap.get(FlashActivity.class));
                intent.setData(uri).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
                activity.startActivity(intent);
            } else {
                SnackbarMaker.showUri(activity, uri);
            }
        } else {
            Utils.toast(R.string.process_error, Toast.LENGTH_LONG);
        }
        super.onPostExecute(result);
    }

    @Override
    public void exec(Void... voids) {
        getActivity().runWithPermission(
                new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, super::exec);
    }

    private class ProgressInputStream extends FilterInputStream {

        ProgressInputStream(InputStream in) {
            super(in);
        }

        private void updateDlProgress(int step) {
            progress += step;
            progressDialog.setMessage(getActivity().getString(R.string.zip_download_msg,
                    (int) (100 * (double) progress / total + 0.5)));
        }

        @Override
        public synchronized int read() throws IOException {
            int b = super.read();
            if (b > 0) {
                Data.mainHandler.post(() -> updateDlProgress(1));
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
                Data.mainHandler.post(() -> updateDlProgress(read));
            }
            return read;
        }
    }
}
