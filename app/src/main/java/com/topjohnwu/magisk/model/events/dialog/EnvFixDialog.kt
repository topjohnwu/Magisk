package com.topjohnwu.magisk.model.events.dialog

import android.content.Context
import android.content.DialogInterface
import android.widget.Toast
import androidx.core.net.toUri
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.cachedFile
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.net.Networking
import com.topjohnwu.magisk.tasks.MagiskInstaller
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.io.SuFile
import org.koin.core.KoinComponent
import org.koin.core.get
import java.io.File

class EnvFixDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) = dialog
        .applyTitle(R.string.env_fix_title)
        .applyMessage(R.string.env_fix_msg)
        .applyButton(MagiskDialog.ButtonType.POSITIVE) {
            titleRes = R.string.yes
            preventDismiss = true
            onClick {
                dialog.applyTitle(R.string.setup_title)
                    .applyMessage(R.string.setup_msg)
                    .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                        title = ""
                    }
                    .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                        title = ""
                    }
                    .cancellable(false)
                fixEnv(it)
            }
        }
        .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
            titleRes = android.R.string.no
        }
        .let { Unit }

    private fun fixEnv(dialog: DialogInterface) {
        object : MagiskInstaller(), KoinComponent {
            override fun operations(): Boolean {
                val context = get<Context>()
                val zip: File = context.cachedFile("magisk.zip")

                installDir = SuFile("/data/adb/magisk")
                Shell.su("rm -rf /data/adb/magisk/*").exec()

                if (!ShellUtils.checkSum("MD5", zip, Info.remote.magisk.md5))
                    Networking.get(Info.remote.magisk.link).execForFile(zip)

                zipUri = zip.toUri()
                return extractZip() && Shell.su("fix_env").exec().isSuccess
            }

            override fun onResult(success: Boolean) {
                dialog.dismiss()
                Utils.toast(
                    if (success) R.string.reboot_delay_toast else R.string.setup_fail,
                    Toast.LENGTH_LONG
                )
                if (success)
                    UiThreadHandler.handler.postDelayed({ reboot() }, 5000)
            }
        }.exec()
    }

}