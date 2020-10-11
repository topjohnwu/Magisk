package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.model.module.OnlineModule
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

@Database(version = 8, entities = [OnlineModule::class], exportSchema = false)
abstract class RepoDatabase : RoomDatabase() {
    abstract fun repoDao() : RepoDao
}

@Dao
abstract class RepoDao(private val db: RepoDatabase) {

    suspend fun clear() = withContext(Dispatchers.IO) { db.clearAllTables() }

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    abstract fun addModule(repo: OnlineModule)

    @Delete
    abstract fun removeModule(repo: OnlineModule)

    @Query("DELETE FROM modules WHERE id = :id")
    abstract fun removeModule(id: String)

    @Query("DELETE FROM modules WHERE id IN (:idList)")
    abstract fun removeModules(idList: Collection<String>)

    @Query("SELECT * FROM modules WHERE id = :id")
    abstract fun getModule(id: String): OnlineModule?

    @Query("SELECT id, last_update FROM modules")
    abstract fun getModuleStubs(): List<ModuleStub>

    fun getModules(offset: Int, limit: Int = LIMIT) = when (Config.repoOrder) {
        Config.Value.ORDER_NAME -> getNameOrder(offset, limit)
        else -> getDateOrder(offset, limit)
    }

    fun searchModules(query: String, offset: Int, limit: Int = LIMIT) = when (Config.repoOrder) {
        Config.Value.ORDER_NAME -> searchNameOrder(query, offset, limit)
        else -> searchDateOrder(query, offset, limit)
    }

    @Query("SELECT * FROM modules WHERE id = :id AND versionCode > :versionCode LIMIT 1")
    abstract fun getUpdatableModule(id: String, versionCode: Int): OnlineModule?

    @Query("SELECT * FROM modules ORDER BY last_update DESC LIMIT :limit OFFSET :offset")
    protected abstract fun getDateOrder(offset: Int, limit: Int): List<OnlineModule>

    @Query("SELECT * FROM modules ORDER BY name COLLATE NOCASE LIMIT :limit OFFSET :offset")
    protected abstract fun getNameOrder(offset: Int, limit: Int): List<OnlineModule>

    @Query(
        """SELECT * 
        FROM modules
        WHERE 
            (author LIKE '%' || :query || '%') ||
            (name LIKE '%' || :query || '%') ||
            (description LIKE '%' || :query || '%')
        ORDER BY last_update DESC
        LIMIT :limit 
        OFFSET :offset"""
    )
    protected abstract fun searchDateOrder(query: String, offset: Int, limit: Int): List<OnlineModule>

    @Query(
        """SELECT * 
        FROM modules
        WHERE 
            (author LIKE '%' || :query || '%') ||
            (name LIKE '%' || :query || '%') ||
            (description LIKE '%' || :query || '%')
        ORDER BY name COLLATE NOCASE
        LIMIT :limit 
        OFFSET :offset"""
    )
    protected abstract fun searchNameOrder(query: String, offset: Int, limit: Int): List<OnlineModule>

    companion object {
        const val LIMIT = 10
    }
}

data class ModuleStub(
    @PrimaryKey val id: String,
    val last_update: Long
)
