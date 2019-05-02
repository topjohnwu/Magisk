package com.topjohnwu.magisk.ui.base

import android.app.Activity
import com.skoumal.teanity.extensions.doOnSubscribeUi
import com.skoumal.teanity.viewmodel.LoadingViewModel
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.utils.Event
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject
import timber.log.Timber


abstract class MagiskViewModel : LoadingViewModel(), Event.AutoListener {

    override fun onEvent(event: Int) = Timber.i("Event of $event was not handled")
    override fun getListeningEvents(): IntArray = intArrayOf()

    fun withView(action: Activity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String): Observable<Boolean> {
        val subject = PublishSubject.create<Boolean>()
        return subject.doOnSubscribeUi { PermissionEvent(permissions.toList(), subject).publish() }
    }

    fun back() = BackPressEvent().publish()

}
