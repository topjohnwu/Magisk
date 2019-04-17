package com.topjohnwu.magisk.ui.base

import android.app.Activity
import com.skoumal.teanity.viewmodel.LoadingViewModel
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.utils.Event
import timber.log.Timber


abstract class MagiskViewModel : LoadingViewModel(), Event.AutoListener {

    override fun onEvent(event: Int) = Timber.i("Event of $event was not handled")
    override fun getListeningEvents(): IntArray = intArrayOf()

    fun withView(action: Activity.() -> Unit) {
        ViewActionEvent(action).publish()
    }

}
