package com.topjohnwu.magisk.ui.theme

import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.dialog.DarkThemeDialog
import com.topjohnwu.magisk.events.RecreateEvent
import com.topjohnwu.magisk.view.TappableHeadlineItem

class ThemeViewModel : BaseViewModel(), TappableHeadlineItem.Listener {

    val themeHeadline = TappableHeadlineItem.ThemeMode

    override fun onItemPressed(item: TappableHeadlineItem) = when (item) {
        is TappableHeadlineItem.ThemeMode -> DarkThemeDialog().show()
    }

    fun saveTheme(theme: Theme) {
        if (!theme.isSelected) {
            Config.themeOrdinal = theme.ordinal
            RecreateEvent().publish()
        }
    }
}
