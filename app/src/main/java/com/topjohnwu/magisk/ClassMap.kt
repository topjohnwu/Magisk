package com.topjohnwu.magisk

import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.model.update.UpdateCheckService
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity

object ClassMap {
    private val map = mapOf(
        App::class.java to a.e::class.java,
        MainActivity::class.java to a.b::class.java,
        SplashActivity::class.java to a.c::class.java,
        FlashActivity::class.java to a.f::class.java,
        UpdateCheckService::class.java to a.g::class.java,
        GeneralReceiver::class.java to a.h::class.java,
        DownloadService::class.java to a.j::class.java,
        SuRequestActivity::class.java to a.m::class.java
    )

    operator fun <T : Class<*>>get(c: Class<*>): T {
        return map.getOrElse(c) { throw IllegalArgumentException() } as T
    }
}
