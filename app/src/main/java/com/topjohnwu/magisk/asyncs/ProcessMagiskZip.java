package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;

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
            try {
                // We might not have busybox yet, unzip with Java
                // We shall have complete busybox after Magisk installation
                File tempdir = new File(magiskManager.getCacheDir(), "magisk");
                ZipUtils.unzip(magiskManager.getContentResolver().openInputStream(mUri), tempdir);
                // Running in parallel mode, open new shell
                Shell.su(true,
                        "rm -f /dev/.magisk",
                        (mBoot != null) ? "echo \"BOOTIMAGE=/dev/block/" + mBoot + "\" >> /dev/.magisk" : "",
                        "echo \"KEEPFORCEENCRYPT=" + String.valueOf(mEnc) + "\" >> /dev/.magisk",
                        "echo \"KEEPVERITY=" + String.valueOf(mVerity) + "\" >> /dev/.magisk",
                        "mkdir -p " + MagiskManager.TMP_FOLDER_PATH,
                        "cp -af " + tempdir + "/. " + MagiskManager.TMP_FOLDER_PATH + "/magisk",
                        "mv -f " + tempdir + "/META-INF " + magiskManager.getCacheDir() + "/META-INF",
                        "rm -rf " + tempdir
                );
            } catch (Exception e) {
                Logger.error("ProcessMagiskZip: Error!");
                e.printStackTrace();
                return false;
            }
            return true;
        }
        return false;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        progressDialog.dismiss();
        if (result) {
            new FlashZip(activity, mUri) {
                @Override
                protected boolean unzipAndCheck() throws Exception {
                    // Don't need to check, as it is downloaded in correct form
                    return true;
                }

                @Override
                protected void onSuccess() {
                    new SerialTask<Void, Void, Void>(activity) {
                        @Override
                        protected Void doInBackground(Void... params) {
                            Shell.su("setprop magisk.version "
                                    + String.valueOf(magiskManager.remoteMagiskVersionCode));
                            magiskManager.updateCheckDone.trigger();
                            return null;
                        }
                    }.exec();
                    super.onSuccess();
                }
            }.exec();
        } else {
            Utils.showUriSnack(activity, mUri);
        }
    }
}
