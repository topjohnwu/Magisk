package com.topjohnwu.magisk.core.base

import android.app.job.JobService
import android.content.Context
import com.topjohnwu.magisk.core.wrap

abstract class BaseJobService : JobService() {
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }
}
