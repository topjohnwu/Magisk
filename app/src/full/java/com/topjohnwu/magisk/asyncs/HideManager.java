package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.PatchAPK;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileOutputStream;

import java.io.FileNotFoundException;
import java.security.SecureRandom;

public class HideManager extends ParallelTask<Void, Void, Boolean> {

    private ProgressDialog dialog;

    public HideManager(Activity activity) {
        super(activity);
    }

    private String genPackageName(String prefix, int length) {
        StringBuilder builder = new StringBuilder(length);
        builder.append(prefix);
        length -= prefix.length();
        SecureRandom random = new SecureRandom();
        String base = "abcdefghijklmnopqrstuvwxyz";
        String alpha = base + base.toUpperCase();
        String full = alpha + "0123456789..........";
        char next, prev = '\0';
        for (int i = 0; i < length; ++i) {
            if (prev == '.' || i == length - 1 || i == 0) {
                next = alpha.charAt(random.nextInt(alpha.length()));
            } else {
                next = full.charAt(random.nextInt(full.length()));
            }
            builder.append(next);
            prev = next;
        }
        return builder.toString();
    }

    @Override
    protected void onPreExecute() {
        dialog = ProgressDialog.show(getActivity(),
                getActivity().getString(R.string.hide_manager_toast),
                getActivity().getString(R.string.hide_manager_toast2));
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();

        // Generate a new app with random package name
        SuFile repack = new SuFile("/data/local/tmp/repack.apk");
        String pkg = genPackageName("com.", Const.ORIG_PKG_NAME.length());

        try {
            if (!PatchAPK.patchPackageID(
                    mm.getPackageCodePath(),
                    new SuFileOutputStream(repack),
                    Const.ORIG_PKG_NAME, pkg))
                return false;
        } catch (FileNotFoundException e) {
            return false;
        }

        // Install the application
        if (!ShellUtils.fastCmdResult(Shell.getShell(), "pm install " + repack))
            return false;

        repack.delete();

        mm.mDB.setStrings(Const.Key.SU_MANAGER, pkg);
        mm.dumpPrefs();
        RootUtils.uninstallPkg(Const.ORIG_PKG_NAME);

        return true;
    }

    @Override
    protected void onPostExecute(Boolean b) {
        dialog.dismiss();
        if (!b) {
            MagiskManager.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG);
        }
        super.onPostExecute(b);
    }
}
