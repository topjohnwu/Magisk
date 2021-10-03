package com.topjohnwu.magisk.core

import android.content.ContextWrapper
import android.content.Intent
import com.topjohnwu.magisk.core.base.BaseReceiver
import com.topjohnwu.magisk.view.Shortcuts

open class Receiver : BaseReceiver() {

    override fun onReceive(context: ContextWrapper, intent: Intent?) {
        intent ?: return

        when (intent.action ?: return) {
            Intent.ACTION_LOCALE_CHANGED -> Shortcuts.setupDynamic(context)
        }
    }
}
