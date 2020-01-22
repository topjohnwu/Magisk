package com.topjohnwu.magisk.core.base

import android.app.Service
import android.content.Context
import com.topjohnwu.magisk.core.wrap
import org.koin.core.KoinComponent

abstract class BaseService : Service(), KoinComponent {
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }
}
