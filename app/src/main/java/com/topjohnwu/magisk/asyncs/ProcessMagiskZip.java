package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

public class ProcessMagiskZip extends ParallelTask<Void, Void, Boolean> {

    private Uri mUri;
    private ProgressDialog progressDialog;
    private String mBoot;
    private boolean mEnc, mVerity;

    public ProcessMagiskZip(Activity context, Uri uri, String boot, boolean enc, boolean verity) {
        super(context);
        mUri = uri;
        mBoot = boot;
        mEnc = enc;
        mVerity = verity;
    }

    @Override
    protected void onPreExecute() {
        progressDialog = ProgressDialog.show(activity,
                activity.getString(R.string.zip_process_title),
                activity.getString(R.string.zip_unzip_msg));
    }

    @Override
    protected Boolean doInBackground(Void... params) {
        if (Shell.rootAccess()) {
            magiskManager.rootShell.su("rm -f /dev/.magisk",
                    (mBoot != null) ? "echo \"BOOTIMAGE=" + mBoot + "\" >> /dev/.magisk" : "",
                    "echo \"KEEPFORCEENCRYPT=" + String.valueOf(mEnc) + "\" >> /dev/.magisk",
                    "echo \"KEEPVERITY=" + String.valueOf(mVerity) + "\" >> /dev/.magisk"
            );
            return true;
        }
        return false;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        progressDialog.dismiss();
        if (result) {
            new FlashZip(activity, mUri).exec();
        } else {
            Utils.showUriSnack(activity, mUri);
        }
        super.onPostExecute(result);
    }
}
