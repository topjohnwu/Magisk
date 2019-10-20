package com.topjohnwu.magisk.redesign.settings

import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.dialog.DarkThemeDialog
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.utils.KObservableField

class SettingsViewModel : CompatViewModel() {

    val redesign = KObservableField(Config.redesign)

    init {
        //todo make observable preference
        redesign.addOnPropertyChangedCallback {
            Config.redesign = redesign.value
            DieEvent().publish()
        }
    }

    fun toggle(item: KObservableField<Boolean>) = item.toggle()
    fun darkModePressed() = DarkThemeDialog().publish()

}