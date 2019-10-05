package com.topjohnwu.magisk.base.viewmodel

import android.app.Activity
import com.github.pwittchen.reactivenetwork.library.rx2.ReactiveNetwork
import com.topjohnwu.magisk.extensions.doOnSubscribeUi
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.utils.KObservableField
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject


abstract class BaseViewModel(
    initialState: State = State.LOADING
) : LoadingViewModel(initialState) {

    val isConnected = KObservableField(false)

    init {
        ReactiveNetwork.observeNetworkConnectivity(get())
            .subscribeK { isConnected.value = it.available() }
            .add()
    }

    fun withView(action: Activity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String): Observable<Boolean> {
        val subject = PublishSubject.create<Boolean>()
        return subject.doOnSubscribeUi { PermissionEvent(permissions.toList(), subject).publish() }
    }

    fun back() = BackPressEvent().publish()

}
