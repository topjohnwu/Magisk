package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Action
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.ktx.res
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

class ManagerInstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        with(dialog) {
            val subject = Subject.Manager(Action.APK.Upgrade)

            applyTitle(R.string.repo_install_title.res(R.string.app_name.res()))
            applyMessage(R.string.repo_install_msg.res(subject.title))

            setCancelable(true)

            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.install
                onClick { DownloadService(context) { this.subject = subject } }
            }

            if (Info.remote.app.note.isEmpty()) return
            applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.app_changelog
                onClick {
                    GlobalScope.launch(Dispatchers.Main.immediate) {
                        MarkDownWindow.show(context, null, Info.remote.app.note)
                    }
                }
            }
        }
    }

}
