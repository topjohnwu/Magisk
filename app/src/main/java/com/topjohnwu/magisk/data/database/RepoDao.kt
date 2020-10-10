package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.model.module.Repo
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

@Database(version = 7, entities = [Repo::class], exportSchema = false)
abstract class RepoDatabase : RoomDatabase() {

    abstract fun repoDao() : RepoDao
    abstract fun repoByUpdatedDao(): RepoByUpdatedDao
    abstract fun repoByNameDao(): RepoByNameDao
}

@Dao
abstract class RepoDao(private val db: RepoDatabase) {

    val repos: List<Repo> get() = when (Config.repoOrder) {
            Config.Value.ORDER_NAME -> getReposNameOrder()
            else -> getReposDateOrder()
        }

    suspend fun clear() = withContext(Dispatchers.IO) { db.clearAllTables() }

    @Query("SELECT * FROM repos ORDER BY last_update DESC")
    protected abstract fun getReposDateOrder(): List<Repo>

    @Query("SELECT * FROM repos ORDER BY name COLLATE NOCASE")
    protected abstract fun getReposNameOrder(): List<Repo>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    abstract fun addRepo(repo: Repo)

    @Query("SELECT * FROM repos WHERE id = :id")
    abstract fun getRepo(id: String): Repo?

    @Query("SELECT id, last_update FROM repos")
    abstract fun getRepoStubs(): List<RepoStub>

    @Delete
    abstract fun removeRepo(repo: Repo)

    @Query("DELETE FROM repos WHERE id = :id")
    abstract fun removeRepo(id: String)

    @Query("DELETE FROM repos WHERE id IN (:idList)")
    abstract fun removeRepos(idList: Collection<String>)
}

data class RepoStub(
    @PrimaryKey val id: String,
    val last_update: Long
)
