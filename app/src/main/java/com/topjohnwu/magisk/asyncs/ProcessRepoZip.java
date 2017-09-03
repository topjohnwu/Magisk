package com.topjohnwu.magisk.asyncs;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class ProcessRepoZip extends ParallelTask<Void, Void, Boolean> {

    private ProgressDialog progressDialog;
    private boolean mInstall;
    private String mLink, mFile;

    public ProcessRepoZip(Activity context, String link, String filename, boolean install) {
        super(context);
        mLink = link;
        mFile = Environment.getExternalStorageDirectory() + "/MagiskManager/" + filename;
        mInstall = install;
    }

    @Override
    protected void onPreExecute() {
        Activity activity = getActivity();
        progressDialog = ProgressDialog.show(activity,
                activity.getString(R.string.zip_download_title),
                activity.getString(R.string.zip_download_msg));
    }

    @Override
    protected void onProgressUpdate(Void... values) {
        progressDialog.setTitle(R.string.zip_process_title);
        progressDialog.setMessage(getActivity().getString(R.string.zip_process_msg));
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        Activity activity = getActivity();
        if (activity == null) return null;
        try {

            // Request zip from Internet
            InputStream in = WebService.request(WebService.GET, mLink, null);
            if (in == null) return false;
            in = new BufferedInputStream(in);

            // Temp files
            File temp1 = new File(activity.getCacheDir(), "1.zip");
            File temp2 = new File(temp1.getParentFile(), "2.zip");
            temp1.getParentFile().mkdir();

            // First remove top folder in Github source zip, Web -> temp1
            ZipUtils.removeTopFolder(in, temp1);

            publishProgress();

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
            Logger.error("ProcessRepoZip: Error!");
            e.printStackTrace();
            return false;
        }
    }

    @Override
    protected void onPostExecute(Boolean result) {
        Activity activity = getActivity();
        if (activity == null) return;
        progressDialog.dismiss();
        Uri uri = Uri.fromFile(new File(mFile));
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
    public ParallelTask<Void, Void, Boolean> exec(Void... voids) {
        Utils.runWithPermission(getActivity(), Manifest.permission.WRITE_EXTERNAL_STORAGE, () -> super.exec(voids));
        return this;
    }
}
