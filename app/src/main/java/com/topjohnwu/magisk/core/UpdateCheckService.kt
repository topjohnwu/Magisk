package com.topjohnwu.magisk.core

import android.content.Context
import androidx.work.*
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.koin.core.KoinComponent
import org.koin.core.inject
import java.util.concurrent.TimeUnit

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

    companion object {
        fun schedule(context: Context) {
            if (Config.checkUpdate) {
                val constraints = Constraints.Builder()
                    .setRequiredNetworkType(NetworkType.CONNECTED)
                    .setRequiresDeviceIdle(true)
                    .build()
                val request = PeriodicWorkRequestBuilder<UpdateCheckService>(12, TimeUnit.HOURS)
                    .setConstraints(constraints)
                    .build()
                WorkManager.getInstance(context).enqueueUniquePeriodicWork(
                    Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID,
                    ExistingPeriodicWorkPolicy.REPLACE, request)
            } else {
                WorkManager.getInstance(context)
                    .cancelUniqueWork(Const.ID.CHECK_MAGISK_UPDATE_WORKER_ID)
            }
        }
    }
}
