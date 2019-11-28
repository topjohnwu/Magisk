package com.topjohnwu.magisk.base.viewmodel

import com.topjohnwu.magisk.base.BaseActivity
import com.topjohnwu.magisk.extensions.doOnSubscribeUi
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.utils.KObservableField
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject
import com.topjohnwu.magisk.Info.isConnected as gIsConnected


abstract class BaseViewModel(
    initialState: State = State.LOADING
) : LoadingViewModel(initialState) {

    val isConnected = object : KObservableField<Boolean>(gIsConnected.value, gIsConnected) {
        override fun get(): Boolean {
            return gIsConnected.value
        }
    }

    fun withView(action: BaseActivity<*, *>.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String): Observable<Boolean> {
        val subject = PublishSubject.create<Boolean>()
        return subject.doOnSubscribeUi { PermissionEvent(permissions.toList(), subject).publish() }
    }

    fun back() = BackPressEvent().publish()

}
