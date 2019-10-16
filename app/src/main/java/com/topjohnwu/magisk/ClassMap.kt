package com.topjohnwu.magisk

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.model.update.UpdateCheckService
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity

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

fun Class<*>.cmp(pkg: String = BuildConfig.APPLICATION_ID): ComponentName {
    val name = ClassMap[this].name
    return ComponentName(pkg, ClassMap.componentMap?.get(name) ?: name)
}

fun Context.intent(c: Class<*>): Intent {
    val cls = ClassMap[c]
    return ClassMap.componentMap?.let {
        val className = it.getOrElse(cls.name) { cls.name }
        Intent().setComponent(ComponentName(this, className))
    } ?: Intent(this, cls)
}
