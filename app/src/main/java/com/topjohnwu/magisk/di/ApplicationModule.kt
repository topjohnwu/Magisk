package com.topjohnwu.magisk.di

import android.content.Context
import androidx.preference.PreferenceManager
import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.model.download.ModuleTransformer
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import org.koin.dsl.module


val applicationModule = module {
    single { RxBus() }
    factory { get<Context>().resources }
    factory { get<Context>() as App }
    factory { get<Context>().packageManager }
    factory(Protected) { get<App>().protectedContext }
    single(SUTimeout) { get<Context>(Protected).getSharedPreferences("su_timeout", 0) }
    single { PreferenceManager.getDefaultSharedPreferences(get<Context>(Protected)) }

    factory { (subject: DownloadSubject) -> ModuleTransformer(get(), subject) }
}