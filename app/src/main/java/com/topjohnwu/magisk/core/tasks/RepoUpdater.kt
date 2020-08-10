package com.topjohnwu.magisk.core.tasks

import com.squareup.moshi.JsonClass
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.ktx.synchronized
import kotlinx.coroutines.*
import timber.log.Timber
import java.net.HttpURLConnection
import java.text.SimpleDateFormat
import java.util.*
import kotlin.collections.HashSet

class RepoUpdater(
    private val api: GithubApiServices,
    private val repoDB: RepoDao
) {

    private fun String.trimEtag() = substring(indexOf('\"'), lastIndexOf('\"') + 1)

    private suspend fun forcedReload(cached: MutableSet<String>) = coroutineScope {
        cached.forEach {
            launch {
                val repo = repoDB.getRepo(it)!!
                try {
                    repo.update()
                    repoDB.addRepo(repo)
                } catch (e: Repo.IllegalRepoException) {
                    Timber.e(e)
                }
            }
        }
    }

    private suspend fun loadRepos(
        repos: List<GithubRepoInfo>,
        cached: MutableSet<String>
    ) = coroutineScope {
        repos.forEach {
            // Skip submission
            if (it.id == "submission")
                return@forEach
            launch {
                val repo = repoDB.getRepo(it.id)?.apply { cached.remove(it.id) } ?: Repo(it.id)
                try {
                    repo.update(it.pushDate)
                    repoDB.addRepo(repo)
                } catch (e: Repo.IllegalRepoException) {
                    Timber.e(e)
                }
            }
        }
    }

    private enum class PageResult {
        SUCCESS,
        CACHED,
        ERROR
    }

    private suspend fun loadPage(
        cached: MutableSet<String>,
        page: Int = 1,
        etag: String = ""
    ): PageResult = coroutineScope {
        runCatching {
            val result = api.fetchRepos(page, etag)
            result.run {
                if (code() == HttpURLConnection.HTTP_NOT_MODIFIED)
                    return@coroutineScope PageResult.CACHED

                if (!isSuccessful)
                    return@coroutineScope PageResult.ERROR

                if (page == 1)
                    repoDB.etagKey = headers()[Const.Key.ETAG_KEY].orEmpty().trimEtag()

                val repoLoad = async { loadRepos(body()!!, cached) }
                val next = if (headers()[Const.Key.LINK_KEY].orEmpty().contains("next")) {
                    async { loadPage(cached, page + 1) }
                } else {
                    async { PageResult.SUCCESS }
                }
                repoLoad.await()
                return@coroutineScope next.await()
            }
        }.getOrElse {
            Timber.e(it)
            PageResult.ERROR
        }
    }

    suspend fun run(forced: Boolean) = withContext(Dispatchers.IO) {
        val cached = HashSet(repoDB.repoIDList).synchronized()
        when (loadPage(cached, etag = repoDB.etagKey)) {
            PageResult.CACHED -> if (forced) forcedReload(cached)
            PageResult.SUCCESS -> repoDB.removeRepos(cached)
            PageResult.ERROR -> Unit
        }
    }
}

private val dateFormat: SimpleDateFormat =
    SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US).apply {
        timeZone = TimeZone.getTimeZone("UTC")
    }

@JsonClass(generateAdapter = true)
data class GithubRepoInfo(
    val name: String,
    val pushed_at: String
) {
    val id get() = name

    @Transient
    val pushDate = dateFormat.parse(pushed_at)!!
}
