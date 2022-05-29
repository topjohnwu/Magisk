package com.topjohnwu.magisk.arch

import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.annotation.SuppressLint
import android.os.Bundle
import androidx.databinding.Bindable
import androidx.databinding.PropertyChangeRegistry
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableHost
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.events.BackPressEvent
import com.topjohnwu.magisk.events.NavigationEvent
import com.topjohnwu.magisk.events.PermissionEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import kotlinx.coroutines.Job

abstract class BaseViewModel(
    initialState: State = State.LOADING
) : ViewModel(), ObservableHost {

    override var callbacks: PropertyChangeRegistry? = null

    enum class State {
        LOADED, LOADING, LOADING_FAILED
    }

    @get:Bindable
    val loading get() = state == State.LOADING
    @get:Bindable
    val loaded get() = state == State.LOADED
    @get:Bindable
    val loadFailed get() = state == State.LOADING_FAILED

    val viewEvents: LiveData<ViewEvent> get() = _viewEvents

    var state= initialState
        set(value) = set(value, field, { field = it }, BR.loading, BR.loaded, BR.loadFailed)

    private val _viewEvents = MutableLiveData<ViewEvent>()
    private var runningJob: Job? = null

    open fun onSaveState(state: Bundle) {}
    open fun onRestoreState(state: Bundle) {}

    /** This should probably never be called manually, it's called manually via delegate. */
    @Synchronized
    fun requestRefresh() {
        if (runningJob?.isActive == true) {
            return
        }
        runningJob = refresh()
    }

    protected open fun refresh(): Job? = null

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

    fun back() = BackPressEvent().publish()

    fun <Event : ViewEvent> Event.publish() {
        _viewEvents.postValue(this)
    }

    fun <Event : ViewEventWithScope> Event.publish() {
        scope = viewModelScope
        _viewEvents.postValue(this)
    }

    fun NavDirections.navigate(pop: Boolean = false) {
        _viewEvents.postValue(NavigationEvent(this, pop))
    }

}
