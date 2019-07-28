package com.topjohnwu.magisk.view.dialogs

import android.net.Uri
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import java.util.*

class MagiskInstallDialog(a: MagiskActivity<*, *>) : CustomAlertDialog(a) {
    init {
        val filename = "Magisk v${Info.remote.magisk.version}" +
                "(${Info.remote.magisk.versionCode})"
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.magisk)))
        setMessage(a.getString(R.string.repo_install_msg, filename))
        setCancelable(true)
        setPositiveButton(R.string.install) { _, _ ->
            val options = ArrayList<String>()
            options.add(a.getString(R.string.download_zip_only))
            options.add(a.getString(R.string.select_patch_file))
            if (Shell.rootAccess()) {
                options.add(a.getString(R.string.direct_install))
                val s = ShellUtils.fastCmd("grep_prop ro.build.ab_update")
                if (s.isNotEmpty() && s.toBoolean()) {
                    options.add(a.getString(R.string.install_inactive_slot))
                }
            }
            InstallMethodDialog(a, options).show()
        }
        if (Info.remote.magisk.note.isNotEmpty()) {
            setNeutralButton(R.string.release_notes) { _, _ ->
                if (Info.remote.magisk.note.contains("forum.xda-developers")) {
                    // Open forum links in browser
                    Utils.openLink(a, Uri.parse(Info.remote.magisk.note))
                } else {
                    MarkDownWindow.show(a, null, Info.remote.magisk.note)
                }
            }
        }
    }
}
