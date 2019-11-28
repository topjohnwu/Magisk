package com.topjohnwu.magisk.model.navigation

import androidx.fragment.app.Fragment
import kotlin.reflect.KClass

interface Navigator {

    //TODO Elevate Fragment to MagiskFragment<*,*> once everything is on board with it
    val baseFragments: List<KClass<out Fragment>>

    fun navigateTo(event: MagiskNavigationEvent)

}