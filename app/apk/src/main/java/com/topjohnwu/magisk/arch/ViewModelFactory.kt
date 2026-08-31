package com.topjohnwu.magisk.arch

import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewmodel.initializer
import androidx.lifecycle.viewmodel.viewModelFactory
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.ui.settings.SettingsViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel

val VMFactory: ViewModelProvider.Factory = viewModelFactory {
    initializer { HomeViewModel(ServiceLocator.networkService) }
    initializer { LogViewModel(ServiceLocator.logRepo) }
    initializer { SuperuserViewModel(ServiceLocator.policyDB) }
    initializer { InstallViewModel(ServiceLocator.networkService) }
    initializer { SuRequestViewModel(ServiceLocator.policyDB, ServiceLocator.timeoutPrefs) }
    initializer { DenyListViewModel() }
    initializer { FlashViewModel() }
    initializer { ActionViewModel() }
    initializer { ModuleViewModel() }
    initializer { SettingsViewModel() }
}
