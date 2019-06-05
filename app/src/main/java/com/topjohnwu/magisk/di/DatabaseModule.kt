package com.topjohnwu.magisk.di

import com.topjohnwu.magisk.data.database.*
import com.topjohnwu.magisk.tasks.UpdateRepos
import org.koin.dsl.module


val databaseModule = module {
    single { LogDao() }
    single { PolicyDao(get()) }
    single { SettingsDao() }
    single { StringDao() }
    single { RepoDatabaseHelper(get()) }
    single { UpdateRepos(get()) }
}
