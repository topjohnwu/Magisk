package com.topjohnwu.magisk.arch

import android.widget.Toast
import androidx.annotation.StringRes
import androidx.lifecycle.ViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.receiveAsFlow

abstract class BaseViewModel : ViewModel() {

    // Keep requests until the UI collects them: one can arrive before the screen's collector
    // has (re)started, e.g. from an activity result delivered right after a recreation
    private val _navEvents = Channel<Route>(Channel.BUFFERED)
    val navEvents: Flow<Route> = _navEvents.receiveAsFlow()

    fun showToast(@StringRes resId: Int) {
        AppContext.toast(resId, Toast.LENGTH_SHORT)
    }

    fun showToast(msg: String) {
        AppContext.toast(msg, Toast.LENGTH_SHORT)
    }

    fun navigateTo(route: Route) {
        _navEvents.trySend(route)
    }
}
