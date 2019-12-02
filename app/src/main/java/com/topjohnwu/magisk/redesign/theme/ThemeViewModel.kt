package com.topjohnwu.magisk.redesign.theme

import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.model.entity.recycler.TappableHeadlineItem
import com.topjohnwu.magisk.model.entity.recycler.ThemeItem
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.model.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf

class ThemeViewModel : CompatViewModel(), TappableHeadlineItem.Listener {

    val items = diffListOf(*Theme.values().map { ThemeItem(it) }.toTypedArray())
    val itemBinding = itemBindingOf<ThemeItem> { it.bindExtra(BR.viewModel, this) }

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