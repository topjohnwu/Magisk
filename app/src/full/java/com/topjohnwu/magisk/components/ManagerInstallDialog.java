package com.topjohnwu.magisk.components;

import android.text.TextUtils;

import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.DownloadApp;

import androidx.annotation.NonNull;

public class ManagerInstallDialog extends CustomAlertDialog {

    public ManagerInstallDialog(@NonNull BaseActivity a) {
        super(a);
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.app_name)));
        setMessage(a.getString(R.string.repo_install_msg, name));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> DownloadApp.upgrade(name));
        setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(Data.managerNoteLink)) {
            setNeutralButton(R.string.app_changelog, (d, i) -> MarkDownWindow.show(a, null, Data.managerNoteLink));
        }
    }
}
