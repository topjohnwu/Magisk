package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.view.MagiskDialog
import org.koin.core.inject

class ManagerInstallDialog : MarkDownDialog() {

    private val svc: NetworkService by inject()

    override suspend fun getMarkdownText() = svc.fetchString(Info.remote.magisk.note)

    override fun build(dialog: MagiskDialog) {
        super.build(dialog)
        with(dialog) {
            setCancelable(true)
            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.install
                onClick { DownloadService.start(context, Subject.Manager()) }
            }
            applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.cancel
            }
        }
    }

}
