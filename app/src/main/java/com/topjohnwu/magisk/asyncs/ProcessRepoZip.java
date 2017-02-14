package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.ByteArrayInOutStream;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

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
        // Create a buffer in memory for input/output
        ByteArrayInOutStream buffer = new ByteArrayInOutStream();

        try {
            // First remove top folder (the folder with the repo name) in Github source zip
            ZipUtils.removeTopFolder(activity.getContentResolver().openInputStream(mUri), buffer);

            // Then sign the zip for the first time
            ZipUtils.signZip(activity, buffer.getInputStream(), buffer, false);

            // Adjust the zip to prevent unzip issues
            ZipUtils.adjustZip(buffer);

            // Finally, sign the whole zip file again
            ZipUtils.signZip(activity, buffer.getInputStream(), buffer, true);

            // Write it back to the downloaded zip
            try (OutputStream out = activity.getContentResolver().openOutputStream(mUri)) {
                buffer.writeTo(out);
            }
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
