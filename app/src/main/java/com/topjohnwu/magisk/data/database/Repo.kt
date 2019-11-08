@file:JvmMultifileClass

package com.topjohnwu.magisk.data.database

import androidx.room.Dao
import androidx.room.Query
import com.topjohnwu.magisk.model.entity.module.Repo

interface RepoBase {

    fun getRepos(offset: Int, limit: Int = 10): List<Repo>

}

@Dao
interface RepoByUpdatedDao : RepoBase {

    @Query("SELECT * FROM repos ORDER BY last_update DESC LIMIT :limit OFFSET :offset")
    override fun getRepos(offset: Int, limit: Int): List<Repo>

}

@Dao
interface RepoByNameDao : RepoBase {

    @Query("SELECT * FROM repos ORDER BY name COLLATE NOCASE LIMIT :limit OFFSET :offset")
    override fun getRepos(offset: Int, limit: Int): List<Repo>

}