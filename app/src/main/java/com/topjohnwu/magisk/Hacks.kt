@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk

import android.annotation.SuppressLint
import android.app.job.JobInfo
import android.app.job.JobScheduler
import android.app.job.JobWorkItem
import android.content.ComponentName
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.res.AssetManager
import android.content.res.Configuration
import android.content.res.Resources
import androidx.annotation.RequiresApi
import androidx.annotation.StringRes
import com.topjohnwu.magisk.extensions.langTagToLocale
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.model.update.UpdateCheckService
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.utils.DynAPK
import com.topjohnwu.magisk.utils.currentLocale
import com.topjohnwu.magisk.utils.defaultLocale
import java.util.*

var isRunningAsStub = false

private val addAssetPath by lazy {
    AssetManager::class.java.getMethod("addAssetPath", String::class.java)
}

fun AssetManager.addAssetPath(path: String) {
    addAssetPath.invoke(this, path)
}

fun Context.wrap(global: Boolean = true): Context
        = if (!global) ResContext(this) else GlobalResContext(this)

fun Context.wrapJob(): Context = object : GlobalResContext(this) {

    @SuppressLint("NewApi")
    override fun getSystemService(name: String): Any? {
        return if (!isRunningAsStub) super.getSystemService(name) else
            when (name) {
                Context.JOB_SCHEDULER_SERVICE ->
                    JobSchedulerWrapper(super.getSystemService(name) as JobScheduler)
                else -> super.getSystemService(name)
            }
    }
}

// Override locale and inject resources from dynamic APK
private fun Resources.patch(config: Configuration = Configuration(configuration)): Resources {
    config.setLocale(currentLocale)
    updateConfiguration(config, displayMetrics)
    if (isRunningAsStub)
        assets.addAssetPath(ResourceMgr.resApk)
    return this
}

fun Class<*>.cmp(pkg: String = BuildConfig.APPLICATION_ID): ComponentName {
    val name = ClassMap[this].name
    return ComponentName(pkg, ClassMap.componentMap?.get(name)
            ?: name)
}

fun Context.intent(c: Class<*>): Intent {
    val cls = ClassMap[c]
    return ClassMap.componentMap?.let {
        val className = it.getOrElse(cls.name) { cls.name }
        Intent().setComponent(ComponentName(this, className))
    } ?: Intent(this, cls)
}

private open class GlobalResContext(base: Context) : ContextWrapper(base) {
    open val mRes: Resources get() = ResourceMgr.resource
    private val loader by lazy { javaClass.classLoader!! }

    override fun getApplicationContext(): Context {
        return this
    }

    override fun getResources(): Resources {
        return mRes
    }

    override fun getClassLoader(): ClassLoader {
        return loader
    }

    override fun createConfigurationContext(config: Configuration): Context {
        return ResContext(super.createConfigurationContext(config))
    }
}

private class ResContext(base: Context) : GlobalResContext(base) {
    override val mRes by lazy { base.resources.patch() }
}

object ResourceMgr {

    internal lateinit var resource: Resources
    internal lateinit var resApk: String

    fun init(context: Context) {
        resource = context.resources
        if (isRunningAsStub)
            resApk = DynAPK.current(context).path
    }

    fun reload(config: Configuration = Configuration(resource.configuration)) {
        val localeConfig = Config.locale
        currentLocale = when {
            localeConfig.isEmpty() -> defaultLocale
            else -> localeConfig.langTagToLocale()
        }
        Locale.setDefault(currentLocale)
        resource.patch(config)
    }

    fun getString(locale: Locale, @StringRes id: Int): String {
        val config = Configuration()
        config.setLocale(locale)
        return Resources(resource.assets, resource.displayMetrics, config).getString(id)
    }

}

@RequiresApi(api = 28)
private class JobSchedulerWrapper(private val base: JobScheduler) : JobScheduler() {

    override fun schedule(job: JobInfo): Int {
        return base.schedule(job.patch())
    }

    override fun enqueue(job: JobInfo, work: JobWorkItem): Int {
        return base.enqueue(job.patch(), work)
    }

    override fun cancel(jobId: Int) {
        base.cancel(jobId)
    }

    override fun cancelAll() {
        base.cancelAll()
    }

    override fun getAllPendingJobs(): List<JobInfo> {
        return base.allPendingJobs
    }

    override fun getPendingJob(jobId: Int): JobInfo? {
        return base.getPendingJob(jobId)
    }

    fun JobInfo.patch(): JobInfo {
        // We need to patch the component of JobInfo to access WorkManager SystemJobService

        val name = service.className
        val component = ComponentName(service.packageName, ClassMap.componentMap?.get(name)
                ?: name)

        // Clone the JobInfo except component
        val builder = JobInfo.Builder(id, component)
                .setExtras(extras)
                .setTransientExtras(transientExtras)
                .setClipData(clipData, clipGrantFlags)
                .setRequiredNetwork(requiredNetwork)
                .setEstimatedNetworkBytes(estimatedNetworkDownloadBytes, estimatedNetworkUploadBytes)
                .setRequiresCharging(isRequireCharging)
                .setRequiresDeviceIdle(isRequireDeviceIdle)
                .setRequiresBatteryNotLow(isRequireBatteryNotLow)
                .setRequiresStorageNotLow(isRequireStorageNotLow)
                .also {
                    triggerContentUris?.let { uris ->
                        for (uri in uris)
                            it.addTriggerContentUri(uri)
                    }
                }
                .setTriggerContentUpdateDelay(triggerContentUpdateDelay)
                .setTriggerContentMaxDelay(triggerContentMaxDelay)
                .setImportantWhileForeground(isImportantWhileForeground)
                .setPrefetch(isPrefetch)
                .setPersisted(isPersisted)

        if (isPeriodic) {
            builder.setPeriodic(intervalMillis, flexMillis)
        } else {
            if (minLatencyMillis > 0)
                builder.setMinimumLatency(minLatencyMillis)
            if (maxExecutionDelayMillis > 0)
                builder.setOverrideDeadline(maxExecutionDelayMillis)
        }
        if (!isRequireDeviceIdle)
            builder.setBackoffCriteria(initialBackoffMillis, backoffPolicy)

        return builder.build()
    }
}

object ClassMap {

    private val classMap = mapOf(
        App::class.java to a.e::class.java,
        MainActivity::class.java to a.b::class.java,
        SplashActivity::class.java to a.c::class.java,
        FlashActivity::class.java to a.f::class.java,
        UpdateCheckService::class.java to a.g::class.java,
        GeneralReceiver::class.java to a.h::class.java,
        DownloadService::class.java to a.j::class.java,
        SuRequestActivity::class.java to a.m::class.java
    )

    // This will be set if running as guest app
    var componentMap: Map<String, String>? = null

    operator fun get(c: Class<*>) = classMap.getOrElse(c) { throw IllegalArgumentException() }
}
