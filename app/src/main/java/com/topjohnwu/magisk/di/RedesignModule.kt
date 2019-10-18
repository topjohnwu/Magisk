package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.redesign.MainViewModel
import com.topjohnwu.magisk.redesign.flash.FlashViewModel
import com.topjohnwu.magisk.redesign.hide.HideViewModel
import com.topjohnwu.magisk.redesign.home.HomeViewModel
import com.topjohnwu.magisk.redesign.log.LogViewModel
import com.topjohnwu.magisk.redesign.module.ModuleViewModel
import com.topjohnwu.magisk.redesign.request.RequestViewModel
import com.topjohnwu.magisk.redesign.safetynet.SafetynetViewModel
import com.topjohnwu.magisk.redesign.settings.SettingsViewModel
import com.topjohnwu.magisk.redesign.superuser.SuperuserViewModel
import com.topjohnwu.magisk.redesign.theme.ThemeViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module

val redesignModule = module {
    viewModel { FlashViewModel() }
    viewModel { HideViewModel() }
    viewModel { HomeViewModel(get()) }
    viewModel { LogViewModel() }
    viewModel { ModuleViewModel() }
    viewModel { RequestViewModel() }
    viewModel { SafetynetViewModel() }
    viewModel { SettingsViewModel() }
    viewModel { SuperuserViewModel() }
    viewModel { ThemeViewModel() }

    viewModel { MainViewModel() }
}