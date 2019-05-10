package com.topjohnwu.magisk.di

import android.content.Context
import androidx.preference.PreferenceManager
import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.App
import org.koin.dsl.module


val applicationModule = module {
    single { RxBus() }
    factory { get<Context>().resources }
    factory { get<Context>() as App }
    factory { get<Context>().packageManager }
    single(SUTimeout) {
        get<App>().protectedContext.getSharedPreferences("su_timeout", 0)
    }
    single { PreferenceManager.getDefaultSharedPreferences(get<App>().protectedContext) }
}
