package com.topjohnwu.magisk.dialog

import android.net.Uri
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.view.MagiskDialog

class LocalModuleInstallDialog(
    private val viewModel: ModuleViewModel,
    private val uri: Uri,
    private val displayName: String
) : DialogBuilder {
    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.confirm_install_title)
            setMessage(context.getString(R.string.confirm_install, displayName))
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                onClick {
                    viewModel.navigateTo(
                        Route.Flash(Const.Value.FLASH_ZIP, uri.toString())
                    )
                }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = android.R.string.cancel
            }
        }
    }
}
