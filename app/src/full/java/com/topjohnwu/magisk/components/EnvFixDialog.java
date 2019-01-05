package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.app.ProgressDialog;

import com.topjohnwu.core.tasks.MagiskInstaller;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import androidx.annotation.NonNull;

import com.sdsmdg.tastytoast.TastyToast;

public class EnvFixDialog extends CustomAlertDialog {

    public EnvFixDialog(@NonNull Activity activity) {
        super(activity);
        setTitle(R.string.env_fix_title);
        setMessage(R.string.env_fix_msg);
        setCancelable(true);
        setPositiveButton(R.string.yes, (d, i) -> {
            ProgressDialog pd = ProgressDialog.show(activity,
                    activity.getString(R.string.setup_title),
                    activity.getString(R.string.setup_msg));
            new MagiskInstaller() {
                @Override
                protected boolean operations() {
                    installDir = new SuFile("/data/adb/magisk");
                    Shell.su("rm -rf /data/adb/magisk/*").exec();
                    return extractZip() && Shell.su("fix_env").exec().isSuccess();
                }

                @Override
                protected void onResult(boolean success) {
                    pd.dismiss();
					if (success){
						TastyToast.makeText(getApplicationContext(), R.string.setup_done, TastyToast.LENGTH_LONG, TastyToast.SUCCESS);
					}else{
						TastyToast.makeText(getApplicationContext(), R.string.setup_fail, TastyToast.LENGTH_LONG, TastyToast.ERROR);
					}
                }
            }.exec();
        });
        setNegativeButton(R.string.no_thanks, null);
    }
}
