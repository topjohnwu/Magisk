package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.DownloadApp
import com.topjohnwu.magisk.view.MarkDownWindow

class ManagerInstallDialog(a: Activity) : CustomAlertDialog(a) {

    init {
        val name = "MagiskManager v${Info.remote.app.version}" +
                "(${Info.remote.app.versionCode})"
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.app_name)))
        setMessage(a.getString(R.string.repo_install_msg, name))
        setCancelable(true)
        setPositiveButton(R.string.install) { _, _ -> DownloadApp.upgrade(name) }
        if (Info.remote.app.note.isNotEmpty()) {
            setNeutralButton(R.string.app_changelog) { _, _ ->
                MarkDownWindow.show(a, null, Info.remote.app.note) }
        }
    }
}
