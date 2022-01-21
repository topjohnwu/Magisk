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
        dialog.apply {

            fun download(install: Boolean) {
                val action = if (install) Action.Flash else Action.Download
                val subject = Subject.Module(item, action)
                DownloadService.start(context, subject)
            }

            setTitle(context.getString(R.string.repo_install_title, item.name))
            setMessage(context.getString(R.string.repo_install_msg, item.downloadFilename))
            setCancelable(true)
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = R.string.download
                icon = R.drawable.ic_download_md2
                onClick { download(false) }
            }

            if (Info.env.isActive) {
                setButton(MagiskDialog.ButtonType.POSITIVE) {
                    text = R.string.install
                    icon = R.drawable.ic_install
                    onClick { download(true) }
                }
            }
        }
    }

}
