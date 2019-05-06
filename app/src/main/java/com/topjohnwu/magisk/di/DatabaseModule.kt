package com.topjohnwu.magisk.di

import android.content.Context
import androidx.room.Room
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.data.database.*
import org.koin.dsl.module


val databaseModule = module {
    single { MagiskDB(get<App>().protectedContext) }
    single { RepoDatabaseHelper(get()) }
    single { createDatabase(get()) }
    single { LogDao() }
    single { PolicyDao(get()) }
    single { SettingsDao() }
    single { StringsDao() }
    single { createRepositoryDao(get()) }
}

fun createDatabase(context: Context): AppDatabase =
    Room.databaseBuilder(context, AppDatabase::class.java, AppDatabase.NAME).build()

fun createRepositoryDao(db: AppDatabase) = db.repoDao()