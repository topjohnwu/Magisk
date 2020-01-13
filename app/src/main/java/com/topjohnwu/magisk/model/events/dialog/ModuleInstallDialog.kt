package com.topjohnwu.magisk.model.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.view.MagiskDialog

class ModuleInstallDialog(private val item: Repo) : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        with(dialog) {

            fun download(install: Boolean) = DownloadService(context) {
                val config = if (install) Configuration.Flash.Primary else Configuration.Download
                subject = DownloadSubject.Module(item, config)
            }

            applyTitle(context.getString(R.string.repo_install_title, item.name))
                .applyMessage(context.getString(R.string.repo_install_msg, item.downloadFilename))
                .cancellable(true)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                    titleRes = R.string.install
                    icon = R.drawable.ic_install
                    onClick { download(true) }
                }
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = R.string.download
                    icon = R.drawable.ic_download_md2
                    onClick { download(false) }
                }
                .reveal()
        }
    }

}
