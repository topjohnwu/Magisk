package com.topjohnwu.magisk.core

import android.content.Context
import androidx.work.CoroutineWorker
import androidx.work.WorkerParameters
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.koin.core.KoinComponent
import org.koin.core.inject

class UpdateCheckService(context: Context, workerParams: WorkerParameters)
    : CoroutineWorker(context, workerParams), KoinComponent {

    private val magiskRepo: MagiskRepository by inject()

    override suspend fun doWork(): Result {
        // Make sure shell initializer was ran
        withContext(Dispatchers.IO) {
            Shell.getShell()
        }
        return magiskRepo.fetchUpdate()?.let {
            if (BuildConfig.VERSION_CODE < it.app.versionCode)
                Notifications.managerUpdate(applicationContext)
            else if (Info.env.isActive && Info.env.magiskVersionCode < it.magisk.versionCode)
                Notifications.magiskUpdate(applicationContext)
            Result.success()
        } ?: Result.failure()
    }
}
