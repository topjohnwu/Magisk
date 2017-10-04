package com.topjohnwu.magisk.asyncs;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
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
    private int progress = 0, total;

    private static final int UPDATE_DL_PROG = 0;
    private static final int SHOW_PROCESSING = 1;

    public ProcessRepoZip(Activity context, String link, String filename, boolean install) {
        super(context);
        mLink = link;
        mFile = new File(Environment.getExternalStorageDirectory() + "/MagiskManager", filename);
        mFile.getParentFile().mkdirs();
        mInstall = install;
    }

    private void removeTopFolder(InputStream in, File output) throws IOException {
        JarInputStream source = new JarInputStream(in);
        JarOutputStream dest = new JarOutputStream(new BufferedOutputStream(new FileOutputStream(output)));
        JarEntry entry;
        String path;
        int size;
        byte buffer[] = new byte[4096];
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
            while((size = source.read(buffer)) != -1) {
                dest.write(buffer, 0, size);
            }
        }
        source.close();
        dest.close();
        in.close();
    }

    @Override
    protected void onPreExecute() {
        Activity activity = getActivity();
        progressDialog = ProgressDialog.show(activity, activity.getString(R.string.zip_download_title), activity.getString(R.string.zip_download_msg, 0));
    }

    @Override
    protected void onProgressUpdate(Object... values) {
        int mode = (int) values[0];
        switch (mode) {
            case UPDATE_DL_PROG:
                int add = (int) values[1];
                progress += add;
                progressDialog.setMessage(getActivity().getString(R.string.zip_download_msg, 100 * progress / total));
                break;
            case SHOW_PROCESSING:
                progressDialog.setTitle(R.string.zip_process_title);
                progressDialog.setMessage(getActivity().getString(R.string.zip_process_msg));
                break;
        }
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        Activity activity = getActivity();
        if (activity == null) return null;
        try {

            // Request zip from Internet
            HttpURLConnection conn = WebService.request(mLink, null);
            if (conn == null) return false;
            total = conn.getContentLength();
            InputStream in = new ProgressUpdateInputStream(conn.getInputStream());

            // Temp files
            File temp1 = new File(activity.getCacheDir(), "1.zip");
            File temp2 = new File(temp1.getParentFile(), "2.zip");
            temp1.getParentFile().mkdir();

            // First remove top folder in Github source zip, Web -> temp1
            removeTopFolder(in, temp1);

            conn.disconnect();
            publishProgress(SHOW_PROCESSING);

            // Then sign the zip for the first time, temp1 -> temp2
            ZipUtils.signZip(activity, temp1, temp2, false);

            // Adjust the zip to prevent unzip issues, temp2 -> temp1
            ZipUtils.zipAdjust(temp2.getPath(), temp1.getPath());

            // Finally, sign the whole zip file again, temp1 -> temp2
            ZipUtils.signZip(activity, temp1, temp2, true);

            // Write it to the target zip, temp2 -> file
            try (OutputStream out = new BufferedOutputStream(new FileOutputStream(mFile));
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
                Intent intent = new Intent(getActivity(), FlashActivity.class);
                intent.setData(uri).putExtra(FlashActivity.SET_ACTION, FlashActivity.FLASH_ZIP);
                activity.startActivity(intent);
            } else {
                Utils.showUriSnack(activity, uri);
            }
        } else {
            Utils.getMagiskManager(activity).toast(R.string.process_error, Toast.LENGTH_LONG);
        }
        super.onPostExecute(result);
    }

    @Override
    public ParallelTask<Void, Object, Boolean> exec(Void... voids) {
        Utils.runWithPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE,
                () -> super.exec(voids));
        return this;
    }

    private class ProgressUpdateInputStream extends BufferedInputStream {

        ProgressUpdateInputStream(@NonNull InputStream in) {
            super(in);
        }

        @Override
        public synchronized int read() throws IOException {
            publishProgress(UPDATE_DL_PROG, 1);
            return super.read();
        }

        @Override
        public int read(@NonNull byte[] b) throws IOException {
            return read(b, 0, b.length);
        }

        @Override
        public synchronized int read(@NonNull byte[] b, int off, int len) throws IOException {
            int read = super.read(b, off, len);
            publishProgress(UPDATE_DL_PROG, read);
            return read;
        }
    }
}
