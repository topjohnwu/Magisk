package com.topjohnwu.magisk.model.navigation

import com.skoumal.teanity.viewevents.NavigationEvent
import com.topjohnwu.magisk.R


object Navigation {

    fun home() = NavigationEvent {
        navDirections { destination = R.id.magiskFragment }
        navOptions { popUpTo = R.id.magiskFragment }
    }

    fun superuser() = NavigationEvent {
        navDirections { destination = R.id.superuserFragment }
    }

    fun modules() = NavigationEvent {
        navDirections { destination = R.id.modulesFragment }
    }

    fun repos() = NavigationEvent {
        navDirections { destination = R.id.reposFragment }
    }

    fun hide() = NavigationEvent {
        navDirections { destination = R.id.magiskHideFragment }
    }

    fun log() = NavigationEvent {
        navDirections { destination = R.id.logFragment }
    }

}
