package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.widget.Toast;

import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.tasks.MagiskInstaller;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import java.io.IOException;

import androidx.annotation.NonNull;

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
                    Utils.toast(success ? R.string.setup_done : R.string.setup_fail, Toast.LENGTH_LONG);
                    if (success) {
                        // Relaunch the app
                        try {
                            Shell.getShell().close();
                        } catch (IOException ignored) {}
                        Intent intent = new Intent(activity, ClassMap.get(SplashActivity.class));
                        intent.addFlags(Intent.FLAG_ACTIVITY_TASK_ON_HOME | Intent.FLAG_ACTIVITY_NEW_TASK);
                        activity.startActivity(intent);
                        activity.finish();
                    }

                }
            }.exec();
        });
        setNegativeButton(R.string.no_thanks, null);
    }
}
