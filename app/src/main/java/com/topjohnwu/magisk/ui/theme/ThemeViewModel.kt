package com.topjohnwu.magisk.ui.theme

import android.view.MenuItem
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.events.RecreateEvent
import com.topjohnwu.magisk.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.view.TappableHeadlineItem

class ThemeViewModel : BaseViewModel(){
    fun onThemeModeOptionClicked(){
        DarkThemeDialog().publish()
    }
    fun saveTheme(theme: Theme) {
        if (!theme.isSelected) {
            Config.themeOrdinal = theme.ordinal
            RecreateEvent().publish()
        }
    }
}