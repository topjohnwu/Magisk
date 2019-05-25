package com.topjohnwu.magisk.data.database

import androidx.room.Dao
import androidx.room.Query
import androidx.room.Transaction
import com.skoumal.teanity.database.BaseDao
import com.topjohnwu.magisk.model.entity.Repository

@Dao
interface RepositoryDao : BaseDao<Repository> {

    @Query("DELETE FROM repos")
    @Transaction
    override fun deleteAll()

    @Query("SELECT * FROM repos ORDER BY lastUpdate DESC")
    override fun fetchAll(): List<Repository>

    @Query("SELECT * FROM repos ORDER BY name ASC")
    fun fetchAllOrderByName(): List<Repository>

}