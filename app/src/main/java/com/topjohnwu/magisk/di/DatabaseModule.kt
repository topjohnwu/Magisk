package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.data.database.RepoDatabaseHelper
import org.koin.dsl.module


val databaseModule = module {
    single { get<App>().DB }
    single { RepoDatabaseHelper(get()) }
}
