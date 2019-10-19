package com.topjohnwu.magisk.model.events.dialog

import android.view.LayoutInflater
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.IncludeInstallOptionsBinding
import com.topjohnwu.magisk.extensions.res
import com.topjohnwu.magisk.model.events.OpenInappLinkEvent
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils

class MagiskInstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        with(dialog) {
            val filename =
                "Magisk v${Info.remote.magisk.version}(${Info.remote.magisk.versionCode})"
            applyTitle(R.string.repo_install_title.res(R.string.magisk.res()))
            applyMessage(R.string.repo_install_msg.res(filename))
            setCancelable(true)
            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.install
                preventDismiss = true
                onClick {
                    updateForInstallMethod(dialog)
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
            applyMessage("")
            applyView(IncludeInstallOptionsBinding.inflate(LayoutInflater.from(dialog.context)))
            applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.download_zip_only
                onClick {
                    preventDismiss = false
                    TODO()
                }
            }
            applyButton(MagiskDialog.ButtonType.NEUTRAL) {
                titleRes = R.string.select_patch_file
                onClick {
                    TODO()
                }
            }
            if (!Shell.rootAccess()) return
            applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.direct_install
                onClick {
                    TODO()
                }
            }
            if (!isABDevice()) return
            applyButton(MagiskDialog.ButtonType.IDGAF) {
                titleRes = R.string.install_inactive_slot
                onClick {
                    TODO()
                }
            }
        }
    }

    private fun isABDevice() = ShellUtils
        .fastCmd("grep_prop ro.build.ab_update")
        .let { it.isNotEmpty() && it.toBoolean() }

}