package com.topjohnwu.magisk.ui.base

import android.app.Activity
import com.github.pwittchen.reactivenetwork.library.rx2.ReactiveNetwork
import com.skoumal.teanity.extensions.doOnSubscribeUi
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.KObservableField
import com.skoumal.teanity.viewmodel.LoadingViewModel
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject


abstract class MagiskViewModel : LoadingViewModel() {

    val isConnected = KObservableField(true)

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
