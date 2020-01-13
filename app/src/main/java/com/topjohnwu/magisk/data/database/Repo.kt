@file:JvmMultifileClass

package com.topjohnwu.magisk.data.database

import androidx.room.Dao
import androidx.room.Query
import com.topjohnwu.magisk.core.model.module.Repo

interface RepoBase {

    fun getRepos(offset: Int, limit: Int = LIMIT): List<Repo>
    fun searchRepos(query: String, offset: Int, limit: Int = LIMIT): List<Repo>

    @Query("SELECT * FROM repos WHERE id = :id AND versionCode > :versionCode LIMIT 1")
    fun getUpdatableRepoById(id: String, versionCode: Int): Repo?

    @Query("SELECT * FROM repos WHERE id = :id LIMIT 1")
    fun getRepoById(id: String): Repo?

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
