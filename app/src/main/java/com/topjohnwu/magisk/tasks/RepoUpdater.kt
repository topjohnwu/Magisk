package com.topjohnwu.magisk.tasks

import com.squareup.moshi.Json
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.model.entity.module.Repo
import io.reactivex.Flowable
import io.reactivex.schedulers.Schedulers
import se.ansman.kotshi.JsonSerializable
import timber.log.Timber
import java.net.HttpURLConnection
import java.text.SimpleDateFormat
import java.util.*
import kotlin.collections.HashSet

class RepoUpdater(
    private val api: GithubApiServices,
    private val repoDB: RepoDao
) {
    private lateinit var cached: MutableSet<String>

    private val dateFormat: SimpleDateFormat
        get() {
            val format = SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US)
            format.timeZone = TimeZone.getTimeZone("UTC")
            return format
        }

    private fun loadRepos(repos: List<GithubRepoInfo>) = Flowable.fromIterable(repos)
            .parallel().runOn(Schedulers.io()).map {
                it.id to dateFormat.parse(it.pushed_at)!!
            }.map {
                // Skip submission
                if (it.first == "submission")
                    return@map
                (repoDB.getRepo(it.first)?.apply {
                    cached.remove(it.first)
                } ?: Repo(it.first)).runCatching {
                    update(it.second)
                    repoDB.addRepo(this)
                }.getOrElse { Timber.e(it) }
            }.sequential()

    private fun loadPage(page: Int, etag: String = ""): Flowable<Unit> =
        api.fetchRepos(page, etag).flatMap {
            it.error()?.also { throw it }
            it.response()?.run {
                if (code() == HttpURLConnection.HTTP_NOT_MODIFIED)
                    throw CachedException()

                if (page == 1)
                    repoDB.etagKey = headers()[Const.Key.ETAG_KEY].orEmpty().trimEtag()

                val flow = loadRepos(body()!!)
                if (headers()[Const.Key.LINK_KEY].orEmpty().contains("next")) {
                    flow.mergeWith(loadPage(page + 1))
                } else {
                    flow
                }
            }
        }

    private fun forcedReload() = Flowable.fromIterable(cached)
            .parallel().runOn(Schedulers.io()).map {
                runCatching {
                    Repo(it).update()
                }.getOrElse { Timber.e(it) }
            }.sequential()

    private fun String.trimEtag() = substring(indexOf('\"'), lastIndexOf('\"') + 1)

    operator fun invoke(forced: Boolean = false) : Flowable<Unit> {
        cached = Collections.synchronizedSet(HashSet(repoDB.repoIDSet))
        return loadPage(1, repoDB.etagKey).doOnComplete {
            repoDB.removeRepos(cached.toList())
            cached.clear()
        }.onErrorResumeNext { it: Throwable ->
            cached.clear()
            if (it is CachedException) {
                if (forced) forcedReload() else Flowable.empty()
            } else {
                Flowable.error(it)
            }
        }
    }

    class CachedException : Exception()
}

@JsonSerializable
data class GithubRepoInfo(
    @Json(name = "name") val id: String,
    val pushed_at: String
)