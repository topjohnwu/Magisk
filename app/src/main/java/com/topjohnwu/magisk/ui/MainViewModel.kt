package com.topjohnwu.magisk.ui

import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.MagiskViewModel


class MainViewModel : MagiskViewModel() {

    fun navPressed() = Navigation.Main.OPEN_NAV.publish()

}
