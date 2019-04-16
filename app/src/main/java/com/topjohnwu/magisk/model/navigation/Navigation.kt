package com.topjohnwu.magisk.model.navigation

import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.MagiskFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment


object Navigation {

    fun home() = MagiskNavigationEvent {
        navDirections { destination = MagiskFragment::class }
        navOptions { popUpTo = MagiskFragment::class }
    }

    fun superuser() = MagiskNavigationEvent {
        navDirections { destination = SuperuserFragment::class }
    }

    fun modules() = MagiskNavigationEvent {
        navDirections { destination = ModulesFragment::class }
    }

    fun repos() = MagiskNavigationEvent {
        navDirections { destination = ReposFragment::class }
    }

    fun hide() = MagiskNavigationEvent {
        navDirections { destination = MagiskHideFragment::class }
    }

    fun log() = MagiskNavigationEvent {
        navDirections { destination = LogFragment::class }
    }

    fun settings() = MagiskNavigationEvent {
        navDirections { destination = SettingsFragment::class }
    }


    object Main {
        const val OPEN_NAV = 1
    }
}
