package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStream;

public class ProcessRepoZip extends ParallelTask<Void, Void, Boolean> {

    private Uri mUri;
    private ProgressDialog progressDialog;

    public ProcessRepoZip(Activity context, Uri uri) {
        super(context);
        mUri = uri;
    }

    @Override
    protected void onPreExecute() {
        progressDialog = ProgressDialog.show(activity,
                activity.getString(R.string.zip_install_progress_title),
                activity.getString(R.string.zip_install_process_zip_msg));
    }

    @Override
    protected Boolean doInBackground(Void... params) {

        FileInputStream in;
        FileOutputStream out;

        try {

            // Create temp file
            File temp1 = new File(magiskManager.getCacheDir(), "1.zip");
            File temp2 = new File(magiskManager.getCacheDir(), "2.zip");
            if (magiskManager.getCacheDir().mkdirs()) {
                temp1.createNewFile();
                temp2.createNewFile();
            }

            out = new FileOutputStream(temp1);

            // First remove top folder in Github source zip, Uri -> temp1
            ZipUtils.removeTopFolder(activity.getContentResolver().openInputStream(mUri), out);
            out.flush();
            out.close();

            out = new FileOutputStream(temp2);

            // Then sign the zip for the first time, temp1 -> temp2
            ZipUtils.signZip(activity, temp1, out, false);
            out.flush();
            out.close();

            // Adjust the zip to prevent unzip issues, temp2 -> temp2
            ZipUtils.adjustZip(temp2);

            out = new FileOutputStream(temp1);

            // Finally, sign the whole zip file again, temp2 -> temp1
            ZipUtils.signZip(activity, temp2, out, true);
            out.flush();
            out.close();

            in = new FileInputStream(temp1);

            // Write it back to the downloaded zip, temp1 -> Uri
            try (OutputStream target = activity.getContentResolver().openOutputStream(mUri)) {
                byte[] buffer = new byte[4096];
                int length;
                if (target == null) throw  new FileNotFoundException();
                while ((length = in.read(buffer)) > 0)
                    target.write(buffer, 0, length);
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
        progressDialog.dismiss();
        if (result) {
            if (Shell.rootAccess())
                new FlashZip(activity, mUri).exec();
            else
                Utils.showUriSnack(activity, mUri);

        } else {
            Toast.makeText(activity, R.string.process_error, Toast.LENGTH_LONG).show();
        }
    }
}
