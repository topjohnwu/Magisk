package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.ShellUtils;

public class RestoreImages extends ParallelTask<Void, Void, Boolean> {

    private ProgressDialog dialog;

    public RestoreImages(Activity activity) {
        super(activity);
    }

    @Override
    protected void onPreExecute() {
        Activity a = getActivity();
        dialog = ProgressDialog.show(a, a.getString(R.string.restore_img), a.getString(R.string.restore_img_msg));
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        return ShellUtils.fastCmdResult("restore_imgs");
    }

    @Override
    protected void onPostExecute(Boolean result) {
        dialog.cancel();
        if (result) {
            MagiskManager.toast(R.string.restore_done, Toast.LENGTH_SHORT);
        } else {
            MagiskManager.toast(R.string.restore_fail, Toast.LENGTH_LONG);
        }
    }
}
