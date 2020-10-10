package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.data.repository.NetworkService
import org.koin.dsl.module


val repositoryModule = module {
    single { LogRepository(get()) }
    single { NetworkService(get(), get(), get(), get()) }
}
