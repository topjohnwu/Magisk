package com.topjohnwu.magisk.model.update

import androidx.work.ListenableWorker
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.model.worker.DelegateWorker
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.Shell

class UpdateCheckService : DelegateWorker() {

    private val magiskRepo: MagiskRepository by inject()

    override fun doWork(): ListenableWorker.Result {
        // Make sure shell initializer was ran
        Shell.getShell()
        return runCatching {
            magiskRepo.fetchUpdate().blockingGet()
            if (BuildConfig.VERSION_CODE < Info.remote.app.versionCode)
                Notifications.managerUpdate(applicationContext)
            else if (Info.magiskVersionCode < Info.remote.magisk.versionCode)
                Notifications.magiskUpdate(applicationContext)
            ListenableWorker.Result.success()
        }.getOrElse {
            ListenableWorker.Result.failure()
        }
    }
}
