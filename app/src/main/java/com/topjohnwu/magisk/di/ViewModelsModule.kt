package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.ui.MainViewModel
import com.topjohnwu.magisk.ui.hide.HideViewModel
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module


val viewModelModules = module {
    viewModel { MainViewModel() }
    viewModel { HomeViewModel(get(), get()) }
    viewModel { SuperuserViewModel(get(), get(), get()) }
    viewModel { HideViewModel(get(), get()) }
}
