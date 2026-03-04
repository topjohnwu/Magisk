package com.topjohnwu.magisk.core

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.Notification
import android.app.job.JobInfo
import android.app.job.JobParameters
import android.app.job.JobScheduler
import android.content.Context
import androidx.core.content.getSystemService
import com.topjohnwu.magisk.core.base.BaseJobService
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.DownloadSession
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.view.Notifications
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.util.concurrent.TimeUnit

class JobService : BaseJobService() {

    private var mSession: Session? = null

    @TargetApi(value = 34)
    inner class Session(
        private var params: JobParameters
    ) : DownloadSession {

        override val context get() = this@JobService
        val engine = DownloadEngine(this)

        fun updateParams(params: JobParameters) {
            this.params = params
            engine.reattach()
        }

        override fun attachNotification(id: Int, builder: Notification.Builder) {
            setNotification(params, id, builder.build(), JOB_END_NOTIFICATION_POLICY_REMOVE)
        }

        override fun onDownloadComplete() {
            jobFinished(params, false)
        }
    }

    @SuppressLint("NewApi")
    override fun onStartJob(params: JobParameters): Boolean {
        return when (params.jobId) {
            Const.ID.CHECK_UPDATE_JOB_ID -> checkUpdate(params)
            Const.ID.DOWNLOAD_JOB_ID -> downloadFile(params)
            else -> false
        }
    }

    override fun onStopJob(params: JobParameters?) = false

    @TargetApi(value = 34)
    private fun downloadFile(params: JobParameters): Boolean {
        params.transientExtras.classLoader = Subject::class.java.classLoader
        val subject = params.transientExtras
            .getParcelable(DownloadEngine.SUBJECT_KEY, Subject::class.java) ?:
            return false

        val session = mSession?.also {
            it.updateParams(params)
        } ?: run {
            Session(params).also { mSession = it }
        }

        session.engine.download(subject)
        return true
    }

    private fun checkUpdate(params: JobParameters): Boolean {
        GlobalScope.launch(Dispatchers.IO) {
            Info.fetchUpdate(ServiceLocator.networkService)?.let {
                if (Info.env.isActive && BuildConfig.APP_VERSION_CODE < it.versionCode)
                    Notifications.updateAvailable()
                jobFinished(params, false)
            }
        }
        return true
    }

    companion object {
        fun schedule(context: Context) {
            val scheduler = context.getSystemService<JobScheduler>() ?: return
            if (Config.checkUpdate) {
                val cmp = JobService::class.java.cmp(context.packageName)
                val info = JobInfo.Builder(Const.ID.CHECK_UPDATE_JOB_ID, cmp)
                    .setPeriodic(TimeUnit.HOURS.toMillis(12))
                    .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                    .setRequiresDeviceIdle(true)
                    .build()
                scheduler.schedule(info)
            } else {
                scheduler.cancel(Const.ID.CHECK_UPDATE_JOB_ID)
            }
        }
    }
}
