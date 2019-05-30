package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.data.repository.*
import org.koin.dsl.module


val repositoryModule = module {
    single { MagiskRepository(get(), get(), get()) }
    single { LogRepository(get()) }
    single { AppRepository(get()) }
    single { SettingRepository(get()) }
    single { StringRepository(get()) }
}
