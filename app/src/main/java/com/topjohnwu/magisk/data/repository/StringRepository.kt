package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.database.StringDao

class StringRepository(private val stringDao: StringDao) {

    fun fetch(key: String) = stringDao.fetch(key)
    fun put(key: String, value: String) = stringDao.put(key, value)
    fun delete(key: String) = stringDao.delete(key)

}