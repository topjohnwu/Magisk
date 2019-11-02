package com.topjohnwu.magisk.di

import android.net.Uri
import com.topjohnwu.magisk.ui.MainViewModel
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.hide.HideViewModel
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module


val viewModelModules = module {
    viewModel { MainViewModel() }
    viewModel { HomeViewModel(get()) }
    viewModel { SuperuserViewModel(get(), get(), get(), get()) }
    viewModel { HideViewModel(get(), get()) }
    viewModel { ModuleViewModel(get(), get(), get()) }
    viewModel { LogViewModel(get(), get()) }
    viewModel { (action: String, file: Uri, additional: Uri) ->
        FlashViewModel(action, file, additional, get())
    }
    viewModel { SuRequestViewModel(get(), get(), get(SUTimeout), get()) }
}
