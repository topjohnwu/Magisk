package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.core.model.su.Converters
import com.topjohnwu.magisk.core.model.su.SuLog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.time.OffsetDateTime

@Database(version = 2, entities = [SuLog::class], exportSchema = false)
@TypeConverters(Converters::class)
abstract class SuLogDatabase : RoomDatabase() {

    abstract fun suLogDao(): SuLogDao
}

@Dao
abstract class SuLogDao(private val db: SuLogDatabase) {

    private val twoWeeksAgo = OffsetDateTime.now().minusWeeks(2)

    suspend fun deleteAll() = withContext(Dispatchers.IO) { db.clearAllTables() }

    suspend fun fetchAll(): MutableList<SuLog> {
        deleteOutdated()
        return fetch()
    }

    @Query("SELECT * FROM logs ORDER BY time DESC")
    protected abstract suspend fun fetch(): MutableList<SuLog>

    @Query("DELETE FROM logs WHERE datetime(time) < datetime(:timeout)")
    protected abstract suspend fun deleteOutdated(timeout: OffsetDateTime = twoWeeksAgo)

    @Insert
    abstract suspend fun insert(log: SuLog)

}
