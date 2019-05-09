package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.data.repository.ModuleRepository
import org.koin.dsl.module


val repositoryModule = module {
    single { MagiskRepository(get(), get(), get()) }
    single { ModuleRepository(get(), get(), get(), get()) }
    single { LogRepository(get()) }
}
