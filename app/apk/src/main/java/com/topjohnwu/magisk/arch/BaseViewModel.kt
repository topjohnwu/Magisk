package com.topjohnwu.magisk.arch

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.annotation.SuppressLint
import android.os.Bundle
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.events.BackPressEvent
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.events.DialogEvent
import com.topjohnwu.magisk.events.PermissionEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow

abstract class BaseViewModel : ViewModel() {

    private val _viewEvents = MutableLiveData<ViewEvent>()
    val viewEvents: LiveData<ViewEvent> get() = _viewEvents

    private val _navEvents = MutableSharedFlow<Route>(extraBufferCapacity = 1)
    val navEvents: SharedFlow<Route> = _navEvents

    open fun onSaveState(state: Bundle) {}
    open fun onRestoreState(state: Bundle) {}
    open fun onNetworkChanged(network: Boolean) {}

    fun withPermission(permission: String, callback: (Boolean) -> Unit) {
        PermissionEvent(permission, callback).publish()
    }

    inline fun withExternalRW(crossinline callback: () -> Unit) {
        withPermission(WRITE_EXTERNAL_STORAGE) {
            if (!it) {
                SnackbarEvent(R.string.external_rw_permission_denied).publish()
            } else {
                callback()
            }
        }
    }

    @SuppressLint("InlinedApi")
    inline fun withInstallPermission(crossinline callback: () -> Unit) {
        withPermission(REQUEST_INSTALL_PACKAGES) {
            if (!it) {
                SnackbarEvent(R.string.install_unknown_denied).publish()
            } else {
                callback()
            }
        }
    }

    @SuppressLint("InlinedApi")
    inline fun withPostNotificationPermission(crossinline callback: () -> Unit) {
        withPermission(POST_NOTIFICATIONS) {
            if (!it) {
                SnackbarEvent(R.string.post_notifications_denied).publish()
            } else {
                callback()
            }
        }
    }

    fun back() = BackPressEvent().publish()

    fun ViewEvent.publish() {
        _viewEvents.postValue(this)
    }

    fun DialogBuilder.show() {
        DialogEvent(this).publish()
    }

    fun navigateTo(route: Route) {
        _navEvents.tryEmit(route)
    }

}
