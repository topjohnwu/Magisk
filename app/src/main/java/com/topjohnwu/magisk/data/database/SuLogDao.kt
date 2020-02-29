package com.topjohnwu.magisk.data.database

import androidx.room.*
import com.topjohnwu.magisk.model.entity.MagiskLog
import io.reactivex.Completable
import io.reactivex.Single
import java.util.*

@Database(version = 1, entities = [MagiskLog::class], exportSchema = false)
abstract class SuLogDatabase : RoomDatabase() {

    abstract fun suLogDao(): SuLogDao
}

@Dao
abstract class SuLogDao(private val db: SuLogDatabase) {

    private val twoWeeksAgo =
        Calendar.getInstance().apply { add(Calendar.WEEK_OF_YEAR, -2) }.timeInMillis

    fun deleteAll() = Completable.fromAction { db.clearAllTables() }

    fun fetchAll() = deleteOutdated().andThen(fetch())

    @Query("SELECT * FROM logs ORDER BY time DESC")
    protected abstract fun fetch(): Single<MutableList<MagiskLog>>

    @Insert
    abstract fun insert(log: MagiskLog): Completable

    @Query("DELETE FROM logs WHERE time < :timeout")
    protected abstract fun deleteOutdated(
        timeout: Long = twoWeeksAgo
    ): Completable

}
