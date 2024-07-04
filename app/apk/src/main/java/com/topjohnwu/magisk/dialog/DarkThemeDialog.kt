package com.topjohnwu.magisk.dialog

import android.app.Activity
import androidx.appcompat.app.AppCompatDelegate
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.view.MagiskDialog

class DarkThemeDialog : DialogBuilder {

    override fun build(dialog: MagiskDialog) {
        val activity = dialog.ownerActivity!!
        dialog.apply {
            setTitle(R.string.settings_dark_mode_title)
            setMessage(R.string.settings_dark_mode_message)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = R.string.settings_dark_mode_light
                icon = R.drawable.ic_day
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_NO, activity) }
            }
            setButton(MagiskDialog.ButtonType.NEUTRAL) {
                text = R.string.settings_dark_mode_system
                icon = R.drawable.ic_day_night
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM, activity) }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = R.string.settings_dark_mode_dark
                icon = R.drawable.ic_night
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_YES, activity) }
            }
        }
    }

    private fun selectTheme(mode: Int, activity: Activity) {
        Config.darkTheme = mode
        (activity as UIActivity<*>).delegate.localNightMode = mode
    }
}
