package com.topjohnwu.magisk.components;

import android.text.TextUtils;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.utils.DlInstallManager;
import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.NonNull;

public class ManagerInstallDialog extends CustomAlertDialog {

    public ManagerInstallDialog(@NonNull BaseActivity activity) {
        super(activity);
        MagiskManager mm = Data.MM();
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);
        setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.app_name)));
        setMessage(mm.getString(R.string.repo_install_msg, name));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> DlInstallManager.upgrade(name));
        setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(Data.managerNoteLink)) {
            setNeutralButton(R.string.app_changelog, (d, i) ->
                    new MarkDownWindow(activity, null, Data.managerNoteLink).exec());
        }
    }
}
