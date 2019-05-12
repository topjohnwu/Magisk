package com.topjohnwu.magisk.di

import android.content.Context
import androidx.room.Room
import com.topjohnwu.magisk.data.database.*
import org.koin.dsl.module


val databaseModule = module {
    single { createDatabase(get()) }
    single { LogDao() }
    single { PolicyDao(get()) }
    single { SettingsDao() }
    single { StringDao() }
    single { createRepositoryDao(get()) }
}

fun createDatabase(context: Context): AppDatabase =
    Room.databaseBuilder(context, AppDatabase::class.java, AppDatabase.NAME).build()

fun createRepositoryDao(db: AppDatabase) = db.repoDao()