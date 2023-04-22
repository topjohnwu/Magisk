package com.topjohnwu.magisk.core.data

import androidx.room.*
import com.topjohnwu.magisk.core.model.su.SuLog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.*

@Database(version = 1, entities = [SuLog::class], exportSchema = false)
abstract class SuLogDatabase : RoomDatabase() {

    abstract fun suLogDao(): SuLogDao
}

@Dao
abstract class SuLogDao(private val db: SuLogDatabase) {

    private val twoWeeksAgo =
        Calendar.getInstance().apply { add(Calendar.WEEK_OF_YEAR, -2) }.timeInMillis

    suspend fun deleteAll() = withContext(Dispatchers.IO) { db.clearAllTables() }

    suspend fun fetchAll(): MutableList<SuLog> {
        deleteOutdated()
        return fetch()
    }

    @Query("SELECT * FROM logs ORDER BY time DESC")
    protected abstract suspend fun fetch(): MutableList<SuLog>

    @Query("DELETE FROM logs WHERE time < :timeout")
    protected abstract suspend fun deleteOutdated(timeout: Long = twoWeeksAgo)

    @Insert
    abstract suspend fun insert(log: SuLog)

}
