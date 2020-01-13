package com.topjohnwu.magisk.model.events.dialog

import android.app.Activity
import androidx.appcompat.app.AppCompatDelegate
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.view.MagiskDialog
import java.lang.ref.WeakReference

class DarkThemeDialog : DialogEvent(), ActivityExecutor {

    private var activity: WeakReference<Activity>? = null

    override fun invoke(activity: BaseActivity) {
        this.activity = WeakReference(activity)
    }

    override fun build(dialog: MagiskDialog) {
        dialog.applyTitle(R.string.settings_dark_mode_title)
            .applyMessage(R.string.settings_dark_mode_message)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.settings_dark_mode_light
                icon = R.drawable.ic_day
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_NO) }
            }
            .applyButton(MagiskDialog.ButtonType.NEUTRAL) {
                titleRes = R.string.settings_dark_mode_system
                icon = R.drawable.ic_day_night
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM) }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.settings_dark_mode_dark
                icon = R.drawable.ic_night
                onClick { selectTheme(AppCompatDelegate.MODE_NIGHT_YES) }
            }
            .onDismiss {
                activity?.clear()
                activity = null
            }
    }

    private fun selectTheme(mode: Int) {
        Config.darkThemeExtended = mode
        activity?.get()?.recreate()
    }


}
