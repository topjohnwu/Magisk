package com.topjohnwu.magisk.model.navigation

import android.content.Context
import android.content.Intent
import android.os.Build
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.intent
import com.topjohnwu.magisk.redesign.install.InstallFragment
import com.topjohnwu.magisk.redesign.safetynet.SafetynetFragment
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.HomeFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.redesign.MainActivity as RedesignActivity
import com.topjohnwu.magisk.redesign.hide.HideFragment as RedesignHideFragment
import com.topjohnwu.magisk.redesign.home.HomeFragment as RedesignHomeFragment
import com.topjohnwu.magisk.redesign.log.LogFragment as RedesignLogFragment
import com.topjohnwu.magisk.redesign.module.ModuleFragment as RedesignModulesFragment
import com.topjohnwu.magisk.redesign.settings.SettingsFragment as RedesignSettingsFragment
import com.topjohnwu.magisk.redesign.superuser.SuperuserFragment as RedesignSuperuserFragment

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
        navDirections {
            destination = when {
                Config.redesign -> RedesignSuperuserFragment::class
                else -> SuperuserFragment::class
            }
        }
    }

    fun modules() = MagiskNavigationEvent {
        navDirections {
            destination = when {
                Config.redesign -> RedesignModulesFragment::class
                else -> ModulesFragment::class
            }
        }
    }

    fun repos() = MagiskNavigationEvent {
        navDirections { destination = ReposFragment::class }
    }

    fun hide() = MagiskNavigationEvent {
        navDirections {
            destination = when {
                Config.redesign -> RedesignHideFragment::class
                else -> MagiskHideFragment::class
            }
        }
    }

    fun safetynet() = MagiskNavigationEvent {
        navDirections { destination = SafetynetFragment::class }
    }

    fun log() = MagiskNavigationEvent {
        navDirections {
            destination = when {
                Config.redesign -> RedesignLogFragment::class
                else -> LogFragment::class
            }
        }
    }

    fun settings() = MagiskNavigationEvent {
        navDirections {
            destination = when {
                Config.redesign -> RedesignSettingsFragment::class
                else -> SettingsFragment::class
            }
        }
    }

    fun install() = MagiskNavigationEvent {
        navDirections { destination = InstallFragment::class }
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
        context.intent(destination)
            .putExtra(Const.Key.OPEN_SECTION, launchIntent.getStringExtra(Const.Key.OPEN_SECTION))
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
