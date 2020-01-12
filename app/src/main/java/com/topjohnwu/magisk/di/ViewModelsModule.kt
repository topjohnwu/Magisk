package com.topjohnwu.magisk.di

import android.net.Uri
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel
import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module


val viewModelModules = module {
    viewModel { (action: String, file: Uri, additional: Uri) ->
        FlashViewModel(action, file, additional, get())
    }
    viewModel { SuRequestViewModel(get(), get(), get(SUTimeout), get()) }
}
