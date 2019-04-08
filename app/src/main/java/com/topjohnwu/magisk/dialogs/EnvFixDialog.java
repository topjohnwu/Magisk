package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.tasks.MagiskInstaller;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;
import com.topjohnwu.superuser.io.SuFile;

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
                    Utils.toast(success ? R.string.reboot_delay_toast : R.string.setup_fail, Toast.LENGTH_LONG);
                    if (success)
                        UiThreadHandler.handler.postDelayed(RootUtils::reboot, 5000);
                }
            }.exec();
        });
        setNegativeButton(R.string.no_thanks, null);
    }
}
