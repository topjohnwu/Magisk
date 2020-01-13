package com.topjohnwu.magisk.model.navigation

import android.content.Context
import android.content.Intent
import android.os.Build
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.hide.HideFragment
import com.topjohnwu.magisk.ui.home.HomeFragment
import com.topjohnwu.magisk.ui.install.InstallFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModuleFragment
import com.topjohnwu.magisk.ui.safetynet.SafetynetFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.ui.theme.ThemeFragment

object Navigation {

    fun home() = MagiskNavigationEvent {
        navDirections {
            destination = HomeFragment::class
        }
        navOptions {
            popUpTo = HomeFragment::class
        }
    }

    fun superuser() = MagiskNavigationEvent {
        navDirections {
            destination = SuperuserFragment::class
        }
    }

    fun modules() = MagiskNavigationEvent {
        navDirections {
            destination = ModuleFragment::class
        }
    }

    fun hide() = MagiskNavigationEvent {
        navDirections {
            destination = HideFragment::class
        }
    }

    fun safetynet() = MagiskNavigationEvent {
        navDirections { destination = SafetynetFragment::class }
    }

    fun log() = MagiskNavigationEvent {
        navDirections {
            destination = LogFragment::class
        }
    }

    fun settings() = MagiskNavigationEvent {
        navDirections {
            destination = SettingsFragment::class
        }
    }

    fun install() = MagiskNavigationEvent {
        navDirections { destination = InstallFragment::class }
    }

    fun theme() = MagiskNavigationEvent {
        navDirections { destination = ThemeFragment::class }
    }

    fun fromSection(section: String) = when (section) {
        "superuser" -> superuser()
        "modules" -> modules()
        "magiskhide" -> hide()
        "log" -> log()
        "settings" -> settings()
        else -> home()
    }

    // redesign starts here

    fun start(launchIntent: Intent, context: Context) {
        context.intent<MainActivity>()
            .putExtra(
                Const.Key.OPEN_SECTION, launchIntent.getStringExtra(
                    Const.Key.OPEN_SECTION))
            .putExtra(
                Const.Key.OPEN_SETTINGS,
                launchIntent.action == ACTION_APPLICATION_PREFERENCES
            )
            .also { context.startActivity(it) }
    }

    object Main {
        const val OPEN_NAV = 1
    }

    private val ACTION_APPLICATION_PREFERENCES
        get() = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            Intent.ACTION_APPLICATION_PREFERENCES
        } else {
            "cannot be null, cannot be empty"
        }
}
