package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.data.repository.AppRepository
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.data.repository.MagiskRepository
import org.koin.dsl.module


val repositoryModule = module {
    single { MagiskRepository(get(), get(), get()) }
    single { LogRepository(get()) }
    single { AppRepository(get()) }
}
