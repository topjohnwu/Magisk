package com.topjohnwu.magisk.ui.base

import android.app.Activity
import com.skoumal.teanity.extensions.doOnSubscribeUi
import com.skoumal.teanity.viewmodel.LoadingViewModel
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject


abstract class MagiskViewModel : LoadingViewModel() {

    fun withView(action: Activity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

    fun withPermissions(vararg permissions: String): Observable<Boolean> {
        val subject = PublishSubject.create<Boolean>()
        return subject.doOnSubscribeUi { PermissionEvent(permissions.toList(), subject).publish() }
    }

    fun back() = BackPressEvent().publish()

}
