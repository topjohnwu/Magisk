package com.topjohnwu.magisk.ui.theme

import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.events.RecreateEvent
import com.topjohnwu.magisk.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.view.TappableHeadlineItem

class ThemeViewModel : BaseViewModel(), TappableHeadlineItem.Listener {

    val themeHeadline = TappableHeadlineItem.ThemeMode

    override fun onItemPressed(item: TappableHeadlineItem) = when (item) {
        is TappableHeadlineItem.ThemeMode -> darkModePressed()
        else -> Unit
    }

    fun saveTheme(theme: Theme) {
        theme.select()
        RecreateEvent().publish()
    }

    private fun darkModePressed() = DarkThemeDialog().publish()

}
