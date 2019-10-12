package com.topjohnwu.magisk.redesign.compat

import android.graphics.Insets
import androidx.annotation.CallSuper
import androidx.databinding.Observable
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import io.reactivex.disposables.Disposable

abstract class CompatViewModel : MagiskViewModel() {

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