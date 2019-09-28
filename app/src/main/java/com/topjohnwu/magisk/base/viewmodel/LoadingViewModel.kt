package com.topjohnwu.magisk.base.viewmodel

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import io.reactivex.*

abstract class LoadingViewModel(defaultState: State = State.LOADING) :
    StatefulViewModel<LoadingViewModel.State>(defaultState) {

    val loading @Bindable get() = state == State.LOADING
    val loaded @Bindable get() = state == State.LOADED
    val loadingFailed @Bindable get() = state == State.LOADING_FAILED

    @Deprecated(
        "Direct access is recommended since 0.2. This access method will be removed in 1.0",
        ReplaceWith("state = State.LOADING", "com.topjohnwu.magisk.base.viewmodel.LoadingViewModel.State"),
        DeprecationLevel.WARNING
    )
    fun setLoading() {
        state = State.LOADING
    }

    @Deprecated(
        "Direct access is recommended since 0.2. This access method will be removed in 1.0",
        ReplaceWith("state = State.LOADED", "com.topjohnwu.magisk.base.viewmodel.LoadingViewModel.State"),
        DeprecationLevel.WARNING
    )
    fun setLoaded() {
        state = State.LOADED
    }

    @Deprecated(
        "Direct access is recommended since 0.2. This access method will be removed in 1.0",
        ReplaceWith("state = State.LOADING_FAILED", "com.topjohnwu.magisk.base.viewmodel.LoadingViewModel.State"),
        DeprecationLevel.WARNING
    )
    fun setLoadingFailed() {
        state = State.LOADING_FAILED
    }

    override fun notifyStateChanged() {
        notifyPropertyChanged(BR.loading)
        notifyPropertyChanged(BR.loaded)
        notifyPropertyChanged(BR.loadingFailed)
    }

    enum class State {
        LOADED, LOADING, LOADING_FAILED
    }

    //region Rx
    protected fun <T> Observable<T>.applyViewModel(viewModel: LoadingViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state = State.LOADING }
            .doOnError { viewModel.state = State.LOADING_FAILED }
            .doOnNext { if (allowFinishing) viewModel.state = State.LOADED }

    protected fun <T> Single<T>.applyViewModel(viewModel: LoadingViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state = State.LOADING }
            .doOnError { viewModel.state = State.LOADING_FAILED }
            .doOnSuccess { if (allowFinishing) viewModel.state = State.LOADED }

    protected fun <T> Maybe<T>.applyViewModel(viewModel: LoadingViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state = State.LOADING }
            .doOnError { viewModel.state = State.LOADING_FAILED }
            .doOnComplete { if (allowFinishing) viewModel.state = State.LOADED }
            .doOnSuccess { if (allowFinishing) viewModel.state = State.LOADED }

    protected fun <T> Flowable<T>.applyViewModel(viewModel: LoadingViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state = State.LOADING }
            .doOnError { viewModel.state = State.LOADING_FAILED }
            .doOnNext { if (allowFinishing) viewModel.state = State.LOADED }

    protected fun Completable.applyViewModel(viewModel: LoadingViewModel, allowFinishing: Boolean = true) =
        doOnSubscribe { viewModel.state = State.LOADING }
            .doOnError { viewModel.state = State.LOADING_FAILED }
            .doOnComplete { if (allowFinishing) viewModel.state = State.LOADED }
    //endregion
}