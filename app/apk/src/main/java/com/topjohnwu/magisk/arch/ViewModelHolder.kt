package com.topjohnwu.magisk.arch

import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.ViewModelStoreOwner
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel

interface ViewModelHolder : LifecycleOwner, ViewModelStoreOwner {

    val viewModel: BaseViewModel

    fun startObserveLiveData() {
        viewModel.viewEvents.observe(this, this::onEventDispatched)
        Info.isConnected.observe(this, viewModel::onNetworkChanged)
    }

    /**
     * Called for all [ViewEvent]s published by associated viewModel.
     */
    fun onEventDispatched(event: ViewEvent) {}
}

object VMFactory : ViewModelProvider.Factory {
    @Suppress("UNCHECKED_CAST")
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        return when (modelClass) {
            HomeViewModel::class.java -> HomeViewModel(ServiceLocator.networkService)
            LogViewModel::class.java -> LogViewModel(ServiceLocator.logRepo)
            SuperuserViewModel::class.java -> SuperuserViewModel(ServiceLocator.policyDB)
            InstallViewModel::class.java ->
                InstallViewModel(ServiceLocator.networkService, ServiceLocator.markwon)
            SuRequestViewModel::class.java ->
                SuRequestViewModel(ServiceLocator.policyDB, ServiceLocator.timeoutPrefs)
            else -> modelClass.newInstance()
        } as T
    }
}

inline fun <reified VM : ViewModel> ViewModelHolder.viewModel() =
    lazy(LazyThreadSafetyMode.NONE) {
        ViewModelProvider(this, VMFactory)[VM::class.java]
    }
