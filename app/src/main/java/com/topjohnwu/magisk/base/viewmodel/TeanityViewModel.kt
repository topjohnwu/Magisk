package com.topjohnwu.magisk.base.viewmodel

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.topjohnwu.magisk.model.events.SimpleViewEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import io.reactivex.disposables.CompositeDisposable
import io.reactivex.disposables.Disposable

abstract class TeanityViewModel : ViewModel() {

    private val disposables = CompositeDisposable()
    private val _viewEvents = MutableLiveData<ViewEvent>()
    val viewEvents: LiveData<ViewEvent> get() = _viewEvents

    override fun onCleared() {
        super.onCleared()
        disposables.clear()
    }

    fun <Event : ViewEvent> Event.publish() {
        _viewEvents.value = this
    }

    fun Int.publish() {
        _viewEvents.value = SimpleViewEvent(this)
    }

    fun Disposable.add() {
        disposables.add(this)
    }
}