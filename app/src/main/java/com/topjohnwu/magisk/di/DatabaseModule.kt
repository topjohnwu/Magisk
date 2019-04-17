package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.App
import org.koin.dsl.module


val databaseModule = module {
    single { get<App>().DB }
}
