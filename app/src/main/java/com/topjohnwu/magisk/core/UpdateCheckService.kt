package com.topjohnwu.magisk.core

import androidx.work.ListenableWorker
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.base.BaseWorkerWrapper
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.superuser.Shell

class UpdateCheckService : BaseWorkerWrapper() {

    private val magiskRepo: MagiskRepository by inject()

    override fun doWork(): ListenableWorker.Result {
        // Make sure shell initializer was ran
        Shell.getShell()
        return runCatching {
            magiskRepo.fetchUpdate().blockingGet()
            if (BuildConfig.VERSION_CODE < Info.remote.app.versionCode)
                Notifications.managerUpdate(applicationContext)
            else if (Info.env.magiskVersionCode < Info.remote.magisk.versionCode)
                Notifications.magiskUpdate(applicationContext)
            ListenableWorker.Result.success()
        }.getOrElse {
            ListenableWorker.Result.failure()
        }
    }
}
