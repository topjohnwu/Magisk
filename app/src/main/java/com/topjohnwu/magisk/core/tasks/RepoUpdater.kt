package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.magisk.core.model.module.Repo
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
            repoDB.getRepoStubs().forEach { map[it.id] = Date(it.last_update) }
        }.synchronized()
        val info = svc.fetchRepoInfo()
        coroutineScope {
            info.modules.forEach {
                launch {
                    val lastUpdated = cachedMap.remove(it.id)
                    if (forced || lastUpdated?.before(Date(it.last_update)) != false) {
                        try {
                            val repo = Repo(it).apply { load() }
                            repoDB.addRepo(repo)
                        } catch (e: Repo.IllegalRepoException) {
                            Timber.e(e)
                        }
                    }
                }
            }
        }
        repoDB.removeRepos(cachedMap.keys)
    }
}
