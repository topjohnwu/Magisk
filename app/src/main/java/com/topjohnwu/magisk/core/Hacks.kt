@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core

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
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.ProcessPhoenix
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.utils.refreshLocale
import com.topjohnwu.magisk.core.utils.updateConfig
import com.topjohnwu.magisk.extensions.forceGetDeclaredField
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.ui.MainActivity

fun AssetManager.addAssetPath(path: String) {
    DynAPK.addAssetPath(this, path)
}

fun Context.wrap(global: Boolean = true): Context =
    if (global) GlobalResContext(this) else ResContext(this)

fun Context.wrapJob(): Context = object : GlobalResContext(this) {

    override fun getApplicationContext(): Context {
        return this
    }

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

fun Class<*>.cmp(pkg: String): ComponentName {
    val name = ClassMap[this].name
    return ComponentName(pkg, Info.stub?.classToComponent?.get(name) ?: name)
}

inline fun <reified T> Context.intent() = Intent().setComponent(T::class.java.cmp(packageName))

private open class GlobalResContext(base: Context) : ContextWrapper(base) {
    open val mRes: Resources get() = ResMgr.resource

    override fun getResources(): Resources {
        return mRes
    }

    override fun getClassLoader(): ClassLoader {
        return javaClass.classLoader!!
    }

    override fun createConfigurationContext(config: Configuration): Context {
        return ResContext(super.createConfigurationContext(config))
    }
}

private class ResContext(base: Context) : GlobalResContext(base) {
    override val mRes by lazy { base.resources.patch() }

    private fun Resources.patch(): Resources {
        updateConfig()
        if (isRunningAsStub)
            assets.addAssetPath(ResMgr.apk)
        return this
    }
}

object ResMgr {

    lateinit var resource: Resources
    lateinit var apk: String

    fun init(context: Context) {
        resource = context.resources
        refreshLocale()
        if (isRunningAsStub) {
            apk = DynAPK.current(context).path
            resource.assets.addAssetPath(apk)
        } else {
            apk = context.packageResourcePath
        }
    }
}

@RequiresApi(28)
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

    private fun JobInfo.patch(): JobInfo {
        // We need to swap out the service of JobInfo
        val name = service.className
        val component = ComponentName(
            service.packageName,
            Info.stub!!.classToComponent[name] ?: name
        )

        javaClass.forceGetDeclaredField("service")?.set(this, component)
        return this
    }
}

private object ClassMap {

    private val map = mapOf(
        App::class.java to a.e::class.java,
        MainActivity::class.java to a.b::class.java,
        SplashActivity::class.java to a.c::class.java,
        GeneralReceiver::class.java to a.h::class.java,
        DownloadService::class.java to a.j::class.java,
        SuRequestActivity::class.java to a.m::class.java,
        ProcessPhoenix::class.java to a.r::class.java
    )

    operator fun get(c: Class<*>) = map.getOrElse(c) { c }
}

/*
* Keep a reference to these resources to prevent it from
* being removed when running "remove unused resources" */
val shouldKeepResources = listOf(
    /* TODO: The following strings should be used somewhere */
    R.string.no_apps_found,
    R.string.no_info_provided,
    R.string.release_notes,
    R.string.settings_download_path_error,
    R.string.invalid_update_channel,
    R.string.update_available
)
