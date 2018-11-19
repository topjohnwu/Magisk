package com.topjohnwu.magisk.components;

import android.app.Activity;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.InstallMagisk;

import androidx.annotation.NonNull;

public class EnvFixDialog extends CustomAlertDialog {

    public EnvFixDialog(@NonNull Activity activity) {
        super(activity);
        setTitle(R.string.env_fix_title);
        setMessage(R.string.env_fix_msg);
        setCancelable(true);
        setPositiveButton(R.string.yes, (d, i) -> new InstallMagisk(activity).exec());
        setNegativeButton(R.string.no_thanks, null);
    }
}
