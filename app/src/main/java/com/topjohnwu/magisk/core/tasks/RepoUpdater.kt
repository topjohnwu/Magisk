package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.ktx.synchronized
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.util.*

class RepoUpdater(
    private val svc: NetworkService,
    private val repoDB: RepoDao
) {

    suspend fun run(forced: Boolean) = withContext(Dispatchers.IO) {
        val cachedMap = HashMap<String, Date>().also { map ->
            repoDB.getModuleStubs().forEach { map[it.id] = Date(it.last_update) }
        }.synchronized()
        svc.fetchRepoInfo()?.let { info ->
            coroutineScope {
                info.modules.forEach {
                    launch {
                        val lastUpdated = cachedMap.remove(it.id)
                        if (forced || lastUpdated?.before(Date(it.last_update)) != false) {
                            try {
                                val repo = OnlineModule(it).apply { load() }
                                repoDB.addModule(repo)
                            } catch (e: OnlineModule.IllegalRepoException) {
                                Timber.e(e)
                            }
                        }
                    }
                }
            }
            repoDB.removeModules(cachedMap.keys)
        }
    }
}
