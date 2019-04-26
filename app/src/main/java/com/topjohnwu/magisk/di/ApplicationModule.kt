package com.topjohnwu.magisk.di

import android.content.Context
import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.App
import org.koin.dsl.module


val applicationModule = module {
    single { RxBus() }
    single { get<Context>().resources }
    single { get<Context>() as App }
    single { get<Context>().packageManager }
    single(SUTimeout) {
        get<App>().protectedContext
            .getSharedPreferences("su_timeout", 0)
    }
}
