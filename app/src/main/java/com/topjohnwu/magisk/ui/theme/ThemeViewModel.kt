package com.topjohnwu.magisk.ui.theme

import com.topjohnwu.magisk.model.entity.recycler.TappableHeadlineItem
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.model.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel

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
