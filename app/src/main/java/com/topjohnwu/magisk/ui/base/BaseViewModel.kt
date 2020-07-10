package com.topjohnwu.magisk.ui.base

import android.Manifest
import androidx.annotation.CallSuper
import androidx.core.graphics.Insets
import androidx.databinding.Bindable
import androidx.databinding.Observable
import androidx.databinding.ObservableField
import androidx.databinding.PropertyChangeRegistry
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.extensions.value
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.navigation.NavigationWrapper
import com.topjohnwu.magisk.model.observer.Observer
import kotlinx.coroutines.Job
import org.koin.core.KoinComponent

abstract class BaseViewModel(
    initialState: State = State.LOADING
) : ViewModel(), Observable, KoinComponent {

    enum class State {
        LOADED, LOADING, LOADING_FAILED
    }

    val loading @Bindable get() = state == State.LOADING
    val loaded @Bindable get() = state == State.LOADED
    val loadingFailed @Bindable get() = state == State.LOADING_FAILED

    val isConnected = Observer(Info.isConnected) { Info.isConnected.value }
    val viewEvents: LiveData<ViewEvent> get() = _viewEvents
    val insets = ObservableField(Insets.NONE)

    var state: State = initialState
        set(value) {
            field = value
            notifyStateChanged()
        }

    private val _viewEvents = MutableLiveData<ViewEvent>()
    private var runningJob: Job? = null
    private val refreshCallback = object : Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
            requestRefresh()
        }
    }

    init {
        isConnected.addOnPropertyChangedCallback(refreshCallback)
    }

    /** This should probably never be called manually, it's called manually via delegate. */
    @Synchronized
    fun requestRefresh() {
        if (runningJob?.isActive == true) {
            return
        }
        runningJob = refresh()
    }

    protected open fun refresh(): Job? = null

    open fun notifyStateChanged() {
        notifyPropertyChanged(BR.loading)
        notifyPropertyChanged(BR.loaded)
        notifyPropertyChanged(BR.loadingFailed)
    }

    @CallSuper
    override fun onCleared() {
        isConnected.removeOnPropertyChangedCallback(refreshCallback)
        super.onCleared()
    }

    fun withView(action: BaseActivity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String, callback: (Boolean) -> Unit) {
        PermissionEvent(permissions.toList(), callback).publish()
    }

    fun withExternalRW(callback: (Boolean) -> Unit) {
        withPermissions(Manifest.permission.WRITE_EXTERNAL_STORAGE, callback = callback)
    }

    fun back() = BackPressEvent().publish()

    fun <Event : ViewEvent> Event.publish() {
        _viewEvents.postValue(this)
    }

    fun <Event : ViewEventsWithScope> Event.publish() {
        scope = viewModelScope
        _viewEvents.postValue(this)
    }

    fun Int.publish() {
        _viewEvents.postValue(SimpleViewEvent(this))
    }

    fun NavDirections.publish() {
        _viewEvents.postValue(NavigationWrapper(this))
    }

    // The following is copied from androidx.databinding.BaseObservable

    @Transient
    private var callbacks: PropertyChangeRegistry? = null

    @Synchronized
    override fun addOnPropertyChangedCallback(callback: Observable.OnPropertyChangedCallback) {
        if (callbacks == null) {
            callbacks = PropertyChangeRegistry()
        }
        callbacks?.add(callback)
    }

    @Synchronized
    override fun removeOnPropertyChangedCallback(callback: Observable.OnPropertyChangedCallback) {
        callbacks?.remove(callback)
    }

    /**
     * Notifies listeners that all properties of this instance have changed.
     */
    @Synchronized
    fun notifyChange() {
        callbacks?.notifyCallbacks(this, 0, null)
    }

    /**
     * Notifies listeners that a specific property has changed. The getter for the property
     * that changes should be marked with [androidx.databinding.Bindable] to generate a field in
     * `BR` to be used as `fieldId`.
     *
     * @param fieldId The generated BR id for the Bindable field.
     */
    fun notifyPropertyChanged(fieldId: Int) {
        callbacks?.notifyCallbacks(this, fieldId, null)
    }

}
