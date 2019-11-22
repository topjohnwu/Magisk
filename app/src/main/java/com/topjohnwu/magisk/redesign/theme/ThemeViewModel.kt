package com.topjohnwu.magisk.redesign.theme

import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.model.entity.recycler.ThemeItem
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.model.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf

class ThemeViewModel : CompatViewModel() {

    val items = diffListOf(*Theme.values().map { ThemeItem(it) }.toTypedArray())
    val itemBinding = itemBindingOf<ThemeItem> { it.bindExtra(BR.viewModel, this) }

    fun saveTheme(theme: Theme) {
        theme.select()
        RecreateEvent().publish()
    }

    fun darkModePressed() = DarkThemeDialog().publish()

}