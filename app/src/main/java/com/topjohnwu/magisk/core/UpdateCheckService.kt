package com.topjohnwu.magisk.core

import android.content.Context
import androidx.work.Worker
import androidx.work.WorkerParameters
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.superuser.Shell

class UpdateCheckService(context: Context, workerParams: WorkerParameters)
    : Worker(context, workerParams) {

    private val magiskRepo: MagiskRepository by inject()

    override fun doWork(): Result {
        // Make sure shell initializer was ran
        Shell.getShell()
        return runCatching {
            magiskRepo.fetchUpdate().blockingGet()
            if (BuildConfig.VERSION_CODE < Info.remote.app.versionCode)
                Notifications.managerUpdate(applicationContext)
            else if (Info.env.magiskVersionCode < Info.remote.magisk.versionCode)
                Notifications.magiskUpdate(applicationContext)
            Result.success()
        }.getOrElse {
            Result.failure()
        }
    }
}
