package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.ui.MainViewModel
import com.topjohnwu.magisk.ui.flash.FlashFragmentArgs
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.hide.HideViewModel
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.ui.safetynet.SafetynetViewModel
import com.topjohnwu.magisk.ui.settings.SettingsViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel
import com.topjohnwu.magisk.ui.theme.ThemeViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module

val viewModelModules = module {
    viewModel { HideViewModel() }
    viewModel { HomeViewModel(get()) }
    viewModel { LogViewModel(get()) }
    viewModel { ModuleViewModel(get(), get()) }
    viewModel { SafetynetViewModel() }
    viewModel { SettingsViewModel(get()) }
    viewModel { SuperuserViewModel(get()) }
    viewModel { ThemeViewModel() }
    viewModel { InstallViewModel(get()) }
    viewModel { MainViewModel() }
    viewModel { (args: FlashFragmentArgs) -> FlashViewModel(args) }
    viewModel { SuRequestViewModel(get(), get(), get(SUTimeout), get()) }
}
