package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.view.MarkDownWindow

class ManagerInstallDialog(a: Activity) : CustomAlertDialog(a) {

    init {
        val subject = DownloadSubject.Manager(Configuration.APK.Upgrade)
        setTitle(a.getString(R.string.repo_install_title, a.getString(R.string.app_name)))
        setMessage(a.getString(R.string.repo_install_msg, subject.title))
        setCancelable(true)
        setPositiveButton(R.string.install) { _, _ ->
            DownloadService(a) { this.subject = subject }
        }
        if (Info.remote.app.note.isNotEmpty()) {
            setNeutralButton(R.string.app_changelog) { _, _ ->
                MarkDownWindow.show(a, null, Info.remote.app.note) }
        }
    }
}
