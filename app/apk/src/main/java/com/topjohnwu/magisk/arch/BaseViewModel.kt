package com.topjohnwu.magisk.arch

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.annotation.SuppressLint
import android.os.Bundle
import androidx.databinding.PropertyChangeRegistry
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.databinding.ObservableHost
import com.topjohnwu.magisk.events.BackPressEvent
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.events.DialogEvent
import com.topjohnwu.magisk.events.NavigationEvent
import com.topjohnwu.magisk.events.PermissionEvent
import com.topjohnwu.magisk.events.SnackbarEvent

abstract class BaseViewModel : ViewModel(), ObservableHost {

    override var callbacks: PropertyChangeRegistry? = null

    private val _viewEvents = MutableLiveData<ViewEvent>()
    val viewEvents: LiveData<ViewEvent> get() = _viewEvents

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

    fun NavDirections.navigate(pop: Boolean = false) {
        _viewEvents.postValue(NavigationEvent(this, pop))
    }

}
