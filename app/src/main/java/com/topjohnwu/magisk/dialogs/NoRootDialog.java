package com.topjohnwu.magisk.dialogs;

import android.app.Activity;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.Const;

public class NoRootDialog extends CustomAlertDialog {

    public NoRootDialog(@NonNull Activity activity) {
        super(activity);
        setTitle(R.string.no_root_title);
        if(Const.USER_ID == 0){
            setMessage(R.string.no_root_msg_user0);
        }else {
            setMessage(R.string.no_root_msg);
        }
        setCancelable(true);
        setPositiveButton(R.string.ok,null);
    }
}
