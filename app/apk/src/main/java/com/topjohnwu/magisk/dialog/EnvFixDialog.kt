package com.topjohnwu.magisk.dialog

import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.view.MagiskDialog
import kotlinx.coroutines.launch

class EnvFixDialog(private val vm: HomeViewModel, private val code: Int) : DialogBuilder {

    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.env_fix_title)
            setMessage(R.string.env_fix_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                doNotDismiss = true
                onClick {
                    dialog.apply {
                        setTitle(R.string.setup_title)
                        setMessage(R.string.setup_msg)
                        resetButtons()
                        setCancelable(false)
                    }
                    dialog.activity.lifecycleScope.launch {
                        MagiskInstaller.FixEnv {
                            dialog.dismiss()
                        }.exec()
                    }
                }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = android.R.string.cancel
            }
        }

        if (code == 2 || // No rules block, module policy not loaded
            Info.env.versionCode != BuildConfig.APP_VERSION_CODE ||
            Info.env.versionString != BuildConfig.APP_VERSION_NAME) {
            dialog.setMessage(R.string.env_full_fix_msg)
            dialog.setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                onClick {
                    vm.onMagiskPressed()
                    dialog.dismiss()
                }
            }
        }
    }
}
