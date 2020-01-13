package com.topjohnwu.magisk.core.base

import android.content.BroadcastReceiver
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import com.topjohnwu.magisk.core.wrap
import org.koin.core.KoinComponent

abstract class BaseReceiver : BroadcastReceiver(), KoinComponent {

    final override fun onReceive(context: Context, intent: Intent?) {
        onReceive(context.wrap() as ContextWrapper, intent)
    }

    abstract fun onReceive(context: ContextWrapper, intent: Intent?)
}
