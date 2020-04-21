package com.topjohnwu.magisk.model.navigation

import android.content.Context
import android.content.Intent
import android.os.Build
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.ui.MainActivity

object Navigation {

    fun start(launchIntent: Intent, context: Context) {
        context.intent<MainActivity>()
            .putExtra(
                Const.Key.OPEN_SECTION, launchIntent.getStringExtra(
                    Const.Key.OPEN_SECTION))
            .putExtra(
                Const.Key.OPEN_SETTINGS,
                launchIntent.action == ACTION_APPLICATION_PREFERENCES
            )
            .also { context.startActivity(it) }
    }

    private val ACTION_APPLICATION_PREFERENCES
        get() = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            Intent.ACTION_APPLICATION_PREFERENCES
        } else {
            "cannot be null, cannot be empty"
        }
}
