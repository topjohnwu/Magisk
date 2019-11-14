@file:JvmMultifileClass

package com.topjohnwu.magisk.data.database

import androidx.room.Dao
import androidx.room.Query
import com.topjohnwu.magisk.model.entity.module.Repo

interface RepoBase {

    fun getRepos(offset: Int, limit: Int = LIMIT): List<Repo>
    fun searchRepos(query: String, offset: Int, limit: Int = LIMIT): List<Repo>

    companion object {
        const val LIMIT = 10
    }

}

@Dao
interface RepoByUpdatedDao : RepoBase {

    @Query("SELECT * FROM repos ORDER BY last_update DESC LIMIT :limit OFFSET :offset")
    override fun getRepos(offset: Int, limit: Int): List<Repo>

    @Query(
        """SELECT * 
        FROM repos
        WHERE 
            (author LIKE '%' || :query || '%') ||
            (name LIKE '%' || :query || '%') ||
            (description LIKE '%' || :query || '%')
        ORDER BY last_update DESC
        LIMIT :limit 
        OFFSET :offset"""
    )
    override fun searchRepos(query: String, offset: Int, limit: Int): List<Repo>

}

@Dao
interface RepoByNameDao : RepoBase {

    @Query("SELECT * FROM repos ORDER BY name COLLATE NOCASE LIMIT :limit OFFSET :offset")
    override fun getRepos(offset: Int, limit: Int): List<Repo>

    @Query(
        """SELECT * 
        FROM repos
        WHERE 
            (author LIKE '%' || :query || '%') ||
            (name LIKE '%' || :query || '%') ||
            (description LIKE '%' || :query || '%')
        ORDER BY name COLLATE NOCASE
        LIMIT :limit 
        OFFSET :offset"""
    )
    override fun searchRepos(query: String, offset: Int, limit: Int): List<Repo>


}