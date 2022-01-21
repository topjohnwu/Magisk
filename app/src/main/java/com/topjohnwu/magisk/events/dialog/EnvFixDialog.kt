package com.topjohnwu.magisk.events.dialog

import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.view.MagiskDialog
import kotlinx.coroutines.launch

class EnvFixDialog(private val vm: HomeViewModel) : DialogEvent() {

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
                    (dialog.ownerActivity as BaseActivity).lifecycleScope.launch {
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

        if (Info.env.versionCode != BuildConfig.VERSION_CODE ||
            Info.env.versionString != BuildConfig.VERSION_NAME) {
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
