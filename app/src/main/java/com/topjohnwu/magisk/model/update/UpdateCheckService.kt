package com.topjohnwu.magisk.model.update

import androidx.work.ListenableWorker
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.model.worker.DelegateWorker
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.view.Notifications

class UpdateCheckService : DelegateWorker() {

    private val magiskRepo: MagiskRepository by inject()

    override fun doWork(): ListenableWorker.Result {
        return runCatching {
            magiskRepo.fetchConfig().blockingGet()
            if (BuildConfig.VERSION_CODE < Config.remoteManagerVersionCode)
                Notifications.managerUpdate()
            else if (Config.magiskVersionCode < Config.remoteManagerVersionCode)
                Notifications.magiskUpdate()
            ListenableWorker.Result.success()
        }.getOrElse {
            ListenableWorker.Result.failure()
        }
    }
}
