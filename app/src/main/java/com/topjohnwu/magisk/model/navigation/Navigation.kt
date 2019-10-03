package com.topjohnwu.magisk.model.navigation

import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.HomeFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.redesign.MainActivity as RedesignActivity
import com.topjohnwu.magisk.redesign.home.HomeFragment as RedesignHomeFragment

object Navigation {

    fun home() = MagiskNavigationEvent {
        navDirections {
            destination = when {
                Config.redesign -> RedesignHomeFragment::class
                else -> HomeFragment::class
            }
        }
        navOptions {
            popUpTo = when {
                Config.redesign -> RedesignHomeFragment::class
                else -> HomeFragment::class
            }
        }
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

    fun fromSection(section: String) = when (section) {
        "superuser" -> superuser()
        "modules" -> modules()
        "downloads" -> repos()
        "magiskhide" -> hide()
        "log" -> log()
        "settings" -> settings()
        else -> home()
    }

    // redesign starts here

    fun start(launchIntent: Intent, context: Context) {
        val destination = when {
            Config.redesign -> RedesignActivity::class.java
            else -> MainActivity::class.java
        }
        val intent = Intent(context, ClassMap[destination])
        intent.putExtra(Const.Key.OPEN_SECTION, launchIntent.getStringExtra(Const.Key.OPEN_SECTION))
        context.startActivity(intent)
    }

    object Main {
        const val OPEN_NAV = 1
    }
}
