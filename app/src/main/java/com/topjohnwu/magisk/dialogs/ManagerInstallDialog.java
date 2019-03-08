package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.uicomponents.MarkDownWindow;
import com.topjohnwu.magisk.utils.DownloadApp;
import com.topjohnwu.magisk.utils.Utils;

public class ManagerInstallDialog extends CustomAlertDialog {

    public ManagerInstallDialog(@NonNull Activity a) {
        super(a);
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Config.remoteManagerVersionString, Config.remoteManagerVersionCode);
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.app_name)));
        setMessage(a.getString(R.string.repo_install_msg, name));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> DownloadApp.upgrade(name));
        if (!TextUtils.isEmpty(Config.managerNoteLink)) {
            setNeutralButton(R.string.app_changelog, (d, i) -> MarkDownWindow.show(a, null, Config.managerNoteLink));
        }
    }
}
