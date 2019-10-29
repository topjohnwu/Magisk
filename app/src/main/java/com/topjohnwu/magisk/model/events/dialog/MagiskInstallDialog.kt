package com.topjohnwu.magisk.model.events.dialog

import android.Manifest
import android.net.Uri
import android.view.LayoutInflater
import android.widget.Toast
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.IncludeInstallOptionsBinding
import com.topjohnwu.magisk.extensions.hasPermissions
import com.topjohnwu.magisk.extensions.res
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils

class MagiskInstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        with(dialog) {
            val filename = "Magisk v%s (%d)".format(
                Info.remote.magisk.version,
                Info.remote.magisk.versionCode
            )
            applyTitle(R.string.repo_install_title.res(R.string.magisk.res()))
            applyMessage(R.string.repo_install_msg.res(filename))
            setCancelable(true)
            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.install
                preventDismiss = true
                onClick {
                    updateForInstallMethod(dialog.reset())
                }
            }

            if (Info.remote.magisk.note.isEmpty()) return
            applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.release_notes
                onClick {
                    if (Info.remote.magisk.note.contains("forum.xda-developers")) {
                        OpenInappLinkEvent(Info.remote.magisk.note).invoke(context)
                    } else {
                        MarkDownWindow.show(context, null, Info.remote.magisk.note)
                    }
                }
            }
        }
    }

    private fun updateForInstallMethod(dialog: MagiskDialog) {
        with(dialog) {
            applyTitle(R.string.select_method)
            applyView(IncludeInstallOptionsBinding.inflate(LayoutInflater.from(dialog.context)))
            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.download_zip_only
                onClick {
                    preventDismiss = false
                    download()
                }
            }
            applyButton(MagiskDialog.ButtonType.NEUTRAL) {
                isEnabled = false
                titleRes = R.string.select_patch_file
                // todo maybe leverage rxbus for this?
                onClick { Utils.toast("This is not currently possible", Toast.LENGTH_LONG) }
            }

            if (!Shell.rootAccess()) return
            applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.direct_install
                onClick { flash() }
            }

            if (!isABDevice()) return
            applyButton(MagiskDialog.ButtonType.IDGAF) {
                titleRes = R.string.install_inactive_slot
                preventDismiss = true
                onClick { inactiveSlotDialog(dialog.reset()) }
            }
        }
    }

    private fun inactiveSlotDialog(dialog: MagiskDialog) {
        dialog.applyTitle(R.string.warning)
            .applyMessage(R.string.install_inactive_slot_msg)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.yes
                onClick {
                    flash(Configuration.Flash.Secondary)
                }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.no
            }
    }

    // ---

    private fun hasPermissions() = dialog.context.hasPermissions(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    private fun patch(data: Uri) = download(DownloadSubject.Magisk(Configuration.Patch(data)))

    private fun flash(
        type: Configuration.Flash = Configuration.Flash.Primary
    ) = download(DownloadSubject.Magisk(type))

    private fun download(
        type: DownloadSubject.Magisk = DownloadSubject.Magisk(Configuration.Download)
    ) {
        if (!hasPermissions()) {
            Utils.toast("Storage permissions are required for this action", Toast.LENGTH_LONG)
            return
        }
        DownloadService(dialog.context) { subject = type }
    }

    // ---

    private fun isABDevice() = ShellUtils
        .fastCmd("grep_prop ro.build.ab_update")
        .let { it.isNotEmpty() && it.toBoolean() }

}