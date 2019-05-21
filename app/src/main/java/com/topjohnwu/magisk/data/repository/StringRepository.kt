package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.database.StringDao

class StringRepository(private val stringDao: StringDao) {

    fun fetch(key: String, default: String) = stringDao.fetch(key, default)
    fun put(key: String, value: String) = stringDao.put(key, value)
    fun delete(key: String) = stringDao.delete(key)

}