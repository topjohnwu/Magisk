package com.topjohnwu.magisk.ui.base

import androidx.annotation.CallSuper
import androidx.core.graphics.Insets
import androidx.databinding.Bindable
import androidx.databinding.PropertyChangeRegistry
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.extensions.doOnSubscribeUi
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.navigation.NavigationWrapper
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.utils.KObservableField
import io.reactivex.*
import io.reactivex.disposables.CompositeDisposable
import io.reactivex.disposables.Disposable
import io.reactivex.subjects.PublishSubject
import org.koin.core.KoinComponent
import androidx.databinding.Observable as BindingObservable

abstract class BaseViewModel(
    initialState: State = State.LOADING
) : ViewModel(), BindingObservable, KoinComponent {

    enum class State {
        LOADED, LOADING, LOADING_FAILED
    }

    val loading @Bindable get() = state == State.LOADING
    val loaded @Bindable get() = state == State.LOADED
    val loadingFailed @Bindable get() = state == State.LOADING_FAILED

    val isConnected = Observer(Info.isConnected) { Info.isConnected.value }
    val viewEvents: LiveData<ViewEvent> get() = _viewEvents
    val insets = KObservableField(Insets.NONE)

    var state: State = initialState
        set(value) {
            field = value
            notifyStateChanged()
        }

    private val disposables = CompositeDisposable()
    private val _viewEvents = MutableLiveData<ViewEvent>()
    private var runningTask: Disposable? = null
    private val refreshCallback = object : androidx.databinding.Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: androidx.databinding.Observable?, propertyId: Int) {
            requestRefresh()
        }
    }

    init {
        isConnected.addOnPropertyChangedCallback(refreshCallback)
    }

    /** This should probably never be called manually, it's called manually via delegate. */
    @Synchronized
    fun requestRefresh() {
        if (runningTask?.isDisposed?.not() == true) {
            return
        }
        runningTask = refresh()
    }

    protected open fun refresh(): Disposable? = null

    open fun notifyStateChanged() {
        notifyPropertyChanged(BR.loading)
        notifyPropertyChanged(BR.loaded)
        notifyPropertyChanged(BR.loadingFailed)
    }

    @CallSuper
    override fun onCleared() {
        isConnected.removeOnPropertyChangedCallback(refreshCallback)
        disposables.clear()
        super.onCleared()
    }

    fun withView(action: BaseActivity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String): Observable<Boolean> {
        val subject = PublishSubject.create<Boolean>()
        return subject.doOnSubscribeUi { PermissionEvent(permissions.toList(), subject).publish() }
    }

    fun back() = BackPressEvent().publish()

    fun <Event : ViewEvent> Event.publish() {
        _viewEvents.postValue(this)
    }

    fun Int.publish() {
        _viewEvents.postValue(SimpleViewEvent(this))
    }

    fun NavDirections.publish() {
        _viewEvents.postValue(NavigationWrapper(this))
    }

    fun Disposable.add() {
        disposables.add(this)
    }

    // The following is copied from androidx.databinding.BaseObservable

    @Transient
    private var callbacks: PropertyChangeRegistry? = null

    @Synchronized
    override fun addOnPropertyChangedCallback(callback: BindingObservable.OnPropertyChangedCallback) {
        if (callbacks == null) {
            callbacks = PropertyChangeRegistry()
        }
        callbacks?.add(callback)
    }

    @Synchronized
    override fun removeOnPropertyChangedCallback(callback: BindingObservable.OnPropertyChangedCallback) {
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

    //region Rx
    protected fun <T> Observable<T>.applyViewModel(viewModel: BaseViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state =
            State.LOADING
        }
            .doOnError { viewModel.state =
                State.LOADING_FAILED
            }
            .doOnNext { if (allowFinishing) viewModel.state =
                State.LOADED
            }

    protected fun <T> Single<T>.applyViewModel(viewModel: BaseViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state =
            State.LOADING
        }
            .doOnError { viewModel.state =
                State.LOADING_FAILED
            }
            .doOnSuccess { if (allowFinishing) viewModel.state =
                State.LOADED
            }

    protected fun <T> Maybe<T>.applyViewModel(viewModel: BaseViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state =
            State.LOADING
        }
            .doOnError { viewModel.state =
                State.LOADING_FAILED
            }
            .doOnComplete { if (allowFinishing) viewModel.state =
                State.LOADED
            }
            .doOnSuccess { if (allowFinishing) viewModel.state =
                State.LOADED
            }

    protected fun <T> Flowable<T>.applyViewModel(viewModel: BaseViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state =
            State.LOADING
        }
            .doOnError { viewModel.state =
                State.LOADING_FAILED
            }
            .doOnNext { if (allowFinishing) viewModel.state =
                State.LOADED
            }

    protected fun Completable.applyViewModel(viewModel: BaseViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state =
            State.LOADING
        }
            .doOnError { viewModel.state =
                State.LOADING_FAILED
            }
            .doOnComplete { if (allowFinishing) viewModel.state =
                State.LOADED
            }
    //endregion
}
