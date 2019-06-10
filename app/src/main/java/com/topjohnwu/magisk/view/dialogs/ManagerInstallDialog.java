package com.topjohnwu.magisk.view.dialogs;

import android.app.Activity;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.Info;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.view.MarkDownWindow;

public class ManagerInstallDialog extends CustomAlertDialog {

    public ManagerInstallDialog(@NonNull Activity a) {
        super(a);
        String name = Utils.INSTANCE.fmt("MagiskManager v%s(%d)",
                Info.remoteManagerVersionString, Info.remoteManagerVersionCode);
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.app_name)));
        setMessage(a.getString(R.string.repo_install_msg, name));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> DownloadApp.upgrade(name));
        if (!TextUtils.isEmpty(Info.managerNoteLink)) {
            setNeutralButton(R.string.app_changelog, (d, i) -> MarkDownWindow.show(a, null, Info.managerNoteLink));
        }
    }
}
