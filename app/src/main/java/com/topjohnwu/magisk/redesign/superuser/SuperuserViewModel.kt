package com.topjohnwu.magisk.redesign.superuser

import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatViewModel

class SuperuserViewModel : CompatViewModel() {

    fun hidePressed() = Navigation.hide().publish()

}