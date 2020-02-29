package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.model.module.Repo

@Database(version = 6, entities = [Repo::class, RepoEtag::class], exportSchema = false)
abstract class RepoDatabase : RoomDatabase() {

    abstract fun repoDao() : RepoDao
    abstract fun repoByUpdatedDao(): RepoByUpdatedDao
    abstract fun repoByNameDao(): RepoByNameDao
}

@Dao
abstract class RepoDao(private val db: RepoDatabase) {

    val repoIDList get() = getRepoID().map { it.id }

    val repos: List<Repo> get() = when (Config.repoOrder) {
            Config.Value.ORDER_NAME -> getReposNameOrder()
            else -> getReposDateOrder()
        }

    var etagKey: String
        set(value) = addEtagRaw(RepoEtag(0, value))
        get() = etagRaw()?.key.orEmpty()

    fun clear() = db.clearAllTables()

    @Query("SELECT * FROM repos ORDER BY last_update DESC")
    protected abstract fun getReposDateOrder(): List<Repo>

    @Query("SELECT * FROM repos ORDER BY name COLLATE NOCASE")
    protected abstract fun getReposNameOrder(): List<Repo>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    abstract fun addRepo(repo: Repo)

    @Query("SELECT * FROM repos WHERE id = :id")
    abstract fun getRepo(id: String): Repo?

    @Query("SELECT id FROM repos")
    protected abstract fun getRepoID(): List<RepoID>

    @Delete
    abstract fun removeRepo(repo: Repo)

    @Query("DELETE FROM repos WHERE id = :id")
    abstract fun removeRepo(id: String)

    @Query("DELETE FROM repos WHERE id IN (:idList)")
    abstract fun removeRepos(idList: Collection<String>)

    @Query("SELECT * FROM etag")
    protected abstract fun etagRaw(): RepoEtag?

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    protected abstract fun addEtagRaw(etag: RepoEtag)
}

data class RepoID(
    @PrimaryKey val id: String
)

@Entity(tableName = "etag")
data class RepoEtag(
    @PrimaryKey val id: Int,
    val key: String
)

