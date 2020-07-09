package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.model.entity.MagiskLog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.*

@Database(version = 1, entities = [MagiskLog::class], exportSchema = false)
abstract class SuLogDatabase : RoomDatabase() {

    abstract fun suLogDao(): SuLogDao
}

@Dao
abstract class SuLogDao(private val db: SuLogDatabase) {

    private val twoWeeksAgo =
        Calendar.getInstance().apply { add(Calendar.WEEK_OF_YEAR, -2) }.timeInMillis

    suspend fun deleteAll() = withContext(Dispatchers.IO) { db.clearAllTables() }

    suspend fun fetchAll(): MutableList<MagiskLog> {
        deleteOutdated()
        return fetch()
    }

    @Query("SELECT * FROM logs ORDER BY time DESC")
    protected abstract suspend fun fetch(): MutableList<MagiskLog>

    @Query("DELETE FROM logs WHERE time < :timeout")
    protected abstract suspend fun deleteOutdated(timeout: Long = twoWeeksAgo)

    @Insert
    abstract suspend fun insert(log: MagiskLog)

}
