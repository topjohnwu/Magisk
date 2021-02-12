package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Action
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.view.MagiskDialog

class ModuleInstallDialog(private val item: OnlineModule) : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        with(dialog) {

            fun download(install: Boolean) {
                val config = if (install) Action.Flash else Action.Download
                val subject = Subject.Module(item, config)
                DownloadService.start(context, subject)
            }

            applyTitle(context.getString(R.string.repo_install_title, item.name))
                .applyMessage(context.getString(R.string.repo_install_msg, item.downloadFilename))
                .cancellable(true)
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = R.string.download
                    icon = R.drawable.ic_download_md2
                    onClick { download(false) }
                }

            if (Info.env.isActive) {
                applyButton(MagiskDialog.ButtonType.POSITIVE) {
                    titleRes = R.string.install
                    icon = R.drawable.ic_install
                    onClick { download(true) }
                }
            }

            reveal()
        }
    }

}
