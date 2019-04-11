package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.ui.MainViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module


val viewModelModules = module {
    viewModel { MainViewModel() }
}
