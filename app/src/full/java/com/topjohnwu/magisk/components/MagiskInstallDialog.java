package com.topjohnwu.magisk.components;

import android.net.Uri;
import android.text.TextUtils;

import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.AppUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.util.ArrayList;
import java.util.List;

public class MagiskInstallDialog extends CustomAlertDialog {
    public MagiskInstallDialog(BaseActivity a) {
        super(a);
        String filename = Utils.fmt("Magisk-v%s(%d).zip",
                Data.remoteMagiskVersionString, Data.remoteMagiskVersionCode);
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.magisk)));
        setMessage(a.getString(R.string.repo_install_msg, filename));
        setCancelable(true);
        setPositiveButton(R.string.install, (d, i) -> {
            List<String> options = new ArrayList<>();
            options.add(a.getString(R.string.download_zip_only));
            options.add(a.getString(R.string.patch_boot_file));
            if (Shell.rootAccess()) {
                options.add(a.getString(R.string.direct_install));
                String s = ShellUtils.fastCmd("grep_prop ro.build.ab_update");
                if (!s.isEmpty() && Boolean.parseBoolean(s)) {
                    options.add(a.getString(R.string.install_inactive_slot));
                }
            }
            new InstallMethodDialog(a, options).show();
        });
        setNegativeButton(R.string.no_thanks, null);
        if (!TextUtils.isEmpty(Data.magiskNoteLink)) {
            setNeutralButton(R.string.release_notes, (d, i) -> {
                if (Data.magiskNoteLink.contains("forum.xda-developers")) {
                    // Open forum links in browser
                    AppUtils.openLink(a, Uri.parse(Data.magiskNoteLink));
                } else {
                    MarkDownWindow.show(a, null, Data.magiskNoteLink);
                }
            });
        }
    }
}
