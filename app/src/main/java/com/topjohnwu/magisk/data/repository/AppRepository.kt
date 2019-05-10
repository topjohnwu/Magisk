package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.database.PolicyDao
import com.topjohnwu.magisk.model.entity.MagiskPolicy

class AppRepository(private val policyDao: PolicyDao) {

    fun deleteOutdated() = policyDao.deleteOutdated()
    fun delete(packageName: String) = policyDao.delete(packageName)
    fun delete(uid: Int) = policyDao.delete(uid)
    fun fetch(uid: Int) = policyDao.fetch(uid)
    fun fetchAll() = policyDao.fetchAll()
    fun update(policy: MagiskPolicy) = policyDao.update(policy)

}