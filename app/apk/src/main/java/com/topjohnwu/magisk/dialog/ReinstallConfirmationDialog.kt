
package com.topjohnwu.magisk.dialog

import android.R as AndroidR
import com.topjohnwu.magisk.core.R as CoreR
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.view.MagiskDialog

class ReinstallConfirmationDialog(
    private val viewModel: ModuleViewModel,
    private val item: OnlineModule
) : DialogBuilder {
    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(CoreR.string.module_reinstall_title)
            setMessage(CoreR.string.module_reinstall_message)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = CoreR.string.module_reinstall
                onClick { viewModel.downloadPressed(item) }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = AndroidR.string.cancel
            }
            setCancelable(true)
        }
    }
} 