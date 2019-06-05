package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.database.SettingsDao

class SettingRepository(private val settingsDao: SettingsDao) {

    fun fetch(key: String, default: Int) = settingsDao.fetch(key, default)
    fun put(key: String, value: Int) = settingsDao.put(key, value)
    fun delete(key: String) = settingsDao.delete(key)

}