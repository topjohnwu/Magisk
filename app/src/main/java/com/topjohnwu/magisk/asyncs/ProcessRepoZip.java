package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.OutputStream;

public class ProcessRepoZip extends ParallelTask<Void, Void, Boolean> {

    private Uri mUri;
    private ProgressDialog progressDialog;
    private boolean mInstall;

    public ProcessRepoZip(Activity context, Uri uri, boolean install) {
        super(context);
        mUri = uri;
        mInstall = install;
    }

    @Override
    protected void onPreExecute() {
        Activity activity = getActivity();
        if (activity == null) return;
        progressDialog = ProgressDialog.show(activity,
                activity.getString(R.string.zip_process_title),
                activity.getString(R.string.zip_process_msg));
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        Activity activity = getActivity();
        if (activity == null) return null;
        try {

            // Create temp file
            File temp1 = new File(activity.getCacheDir(), "1.zip");
            File temp2 = new File(activity.getCacheDir(), "2.zip");
            activity.getCacheDir().mkdirs();
            temp1.createNewFile();
            temp2.createNewFile();

            // First remove top folder in Github source zip, Uri -> temp1
            ZipUtils.removeTopFolder(activity.getContentResolver().openInputStream(mUri), temp1);

            // Then sign the zip for the first time, temp1 -> temp2
            ZipUtils.signZip(activity, temp1, temp2, false);

            // Adjust the zip to prevent unzip issues, temp2 -> temp1
            ZipUtils.zipAdjust(temp2.getPath(), temp1.getPath());

            // Finally, sign the whole zip file again, temp1 -> temp2
            ZipUtils.signZip(activity, temp1, temp2, true);

            // Write it back to the downloaded zip, temp2 -> Uri
            try (OutputStream out = activity.getContentResolver().openOutputStream(mUri);
                 FileInputStream in = new FileInputStream(temp2)
            ) {
                byte[] buffer = new byte[4096];
                int length;
                if (out == null) throw new FileNotFoundException();
                while ((length = in.read(buffer)) > 0)
                    out.write(buffer, 0, length);
            }

            // Delete the temp file
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
        if (result) {
            if (Shell.rootAccess() && mInstall) {
                activity.startActivity(new Intent(activity, FlashActivity.class).setData(mUri));
            } else {
                Utils.showUriSnack(activity, mUri);
            }
        } else {
            Utils.getMagiskManager(activity).toast(R.string.process_error, Toast.LENGTH_LONG);
        }
    }
}
