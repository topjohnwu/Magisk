package com.topjohnwu.magisk.ui

import android.view.MenuItem
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.MagiskViewModel


class MainViewModel : MagiskViewModel() {

    fun navPressed() = Navigation.Main.OPEN_NAV.publish()

    fun navigationItemPressed(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.magiskFragment -> Navigation.home()
            R.id.superuserFragment -> Navigation.superuser()
            R.id.magiskHideFragment -> Navigation.hide()
            R.id.modulesFragment -> Navigation.modules()
            R.id.reposFragment -> Navigation.repos()
            R.id.logFragment -> Navigation.log()
            R.id.settings -> Navigation.settings()
            else -> null
        }?.publish()?.let { return@navigationItemPressed true }
        return false
    }

}
