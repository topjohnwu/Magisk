package com.topjohnwu.magisk.dialogs;

import android.net.Uri;
import android.text.TextUtils;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.uicomponents.MarkDownWindow;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.util.ArrayList;
import java.util.List;

public class MagiskInstallDialog extends CustomAlertDialog {
    public MagiskInstallDialog(BaseActivity a) {
        super(a);
        String filename = Utils.fmt("Magisk-v%s(%d).zip",
                Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode);
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
        if (!TextUtils.isEmpty(Config.magiskNoteLink)) {
            setNeutralButton(R.string.release_notes, (d, i) -> {
                if (Config.magiskNoteLink.contains("forum.xda-developers")) {
                    // Open forum links in browser
                    Utils.openLink(a, Uri.parse(Config.magiskNoteLink));
                } else {
                    MarkDownWindow.show(a, null, Config.magiskNoteLink);
                }
            });
        }
    }
}
