package com.topjohnwu.magisk.components;

import android.Manifest;
import android.content.Intent;
import android.text.TextUtils;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.receivers.ManagerUpdate;
import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.NonNull;

public class ManagerInstallDialog extends CustomAlertDialog {

    public ManagerInstallDialog(@NonNull BaseActivity activity) {
        super(activity);
        MagiskManager mm = Data.MM();
        String filename = Utils.fmt("MagiskManager-v%s(%d).apk",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);
        setTitle(mm.getString(R.string.repo_install_title, mm.getString(R.string.app_name)));
        setMessage(mm.getString(R.string.repo_install_msg, filename));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> activity.runWithPermission(
                new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
                    Intent intent = new Intent(mm, Data.classMap.get(ManagerUpdate.class));
                    intent.putExtra(Const.Key.INTENT_SET_LINK, Data.managerLink);
                    intent.putExtra(Const.Key.INTENT_SET_FILENAME, filename);
                    mm.sendBroadcast(intent);
                }))
                .setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(Data.managerNoteLink)) {
            setNeutralButton(R.string.app_changelog, (d, i) ->
                    new MarkDownWindow(activity, null, Data.managerNoteLink).exec());
        }
    }
}
