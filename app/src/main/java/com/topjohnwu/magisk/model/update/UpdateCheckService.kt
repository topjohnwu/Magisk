package com.topjohnwu.magisk.model.update

import androidx.work.ListenableWorker
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.model.entity.MagiskConfig
import com.topjohnwu.magisk.model.worker.DelegateWorker
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.view.Notifications

class UpdateCheckService : DelegateWorker() {

    private val magiskRepo: MagiskRepository by inject()

    override fun doWork(): ListenableWorker.Result {
        val config = runCatching { magiskRepo.fetchConfig().blockingGet() }
        config.getOrNull()?.let { checkUpdates(it) }
        return when {
            config.isFailure -> ListenableWorker.Result.failure()
            else -> ListenableWorker.Result.success()
        }
    }

    private fun checkUpdates(config: MagiskConfig) {
        when {
            BuildConfig.VERSION_CODE < config.app.versionCode.toIntOrNull() ?: -1 -> Notifications.managerUpdate()
            Config.magiskVersionCode < config.magisk.versionCode.toIntOrNull() ?: -1 -> Notifications.magiskUpdate()
        }
    }
}
