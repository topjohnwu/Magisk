package com.topjohnwu.magisk.redesign.settings

import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.utils.KObservableField

class SettingsViewModel : CompatViewModel() {

    val redesign = KObservableField(Config.redesign)
    val darkTheme = KObservableField(Config.darkTheme)

    init {
        //todo make observable preference
        redesign.addOnPropertyChangedCallback {
            Config.redesign = redesign.value
            DieEvent().publish()
        }
        darkTheme.addOnPropertyChangedCallback {
            Config.darkTheme = darkTheme.value
            RecreateEvent().publish()
        }
    }

    fun toggle(item: KObservableField<Boolean>) = item.toggle()

}