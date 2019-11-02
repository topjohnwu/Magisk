package com.topjohnwu.magisk.data.database

import androidx.room.Database
import androidx.room.RoomDatabase
import com.topjohnwu.magisk.model.entity.module.Repo

@Database(version = 6, entities = [Repo::class, RepoEtag::class])
abstract class RepoDatabase : RoomDatabase() {

    abstract fun repoDao() : RepoDao
}
