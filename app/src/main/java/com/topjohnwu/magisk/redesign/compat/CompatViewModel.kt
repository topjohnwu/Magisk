package com.topjohnwu.magisk.redesign.compat

import androidx.annotation.CallSuper
import androidx.core.graphics.Insets
import androidx.databinding.Observable
import com.topjohnwu.magisk.base.viewmodel.BaseViewModel
import com.topjohnwu.magisk.utils.KObservableField
import io.reactivex.disposables.Disposable
import org.koin.core.KoinComponent

abstract class CompatViewModel(
    initialState: State = State.LOADING
) : BaseViewModel(initialState), KoinComponent {

    val insets = KObservableField(Insets.NONE)

    private var runningTask: Disposable? = null
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
        if (runningTask?.isDisposed?.not() == true) {
            return
        }
        runningTask = refresh()
    }

    protected open fun refresh(): Disposable? = null

    @CallSuper
    override fun onCleared() {
        isConnected.removeOnPropertyChangedCallback(refreshCallback)
        super.onCleared()
    }

}
