package com.topjohnwu.magisk.data.repository

import android.content.Context
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.data.database.RepositoryDao
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubRawApiServices
import com.topjohnwu.magisk.data.network.GithubServices
import com.topjohnwu.magisk.model.entity.GithubRepo
import com.topjohnwu.magisk.model.entity.toRepository
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.toSingle
import com.topjohnwu.magisk.utils.writeToFile
import com.topjohnwu.magisk.utils.writeToString
import io.reactivex.Completable
import io.reactivex.Single

class ModuleRepository(
    private val context: Context,
    private val apiRaw: GithubRawApiServices,
    private val api: GithubApiServices,
    private val apiWeb: GithubServices,
    private val repoDao: RepositoryDao
) {

    fun fetchModules(force: Boolean = false) =
        if (force) fetchRemoteRepos() else fetchCachedOrdered()
            .flatMap { if (it.isEmpty()) fetchRemoteRepos() else it.toSingle() }

    private fun fetchRemoteRepos() = fetchAllRepos()
        .map {
            it.mapNotNull {
                runCatching {
                    fetchProperties(it.name, it.updatedAtMillis).blockingGet()
                }.getOrNull()
            }
        }
        //either github apparently returns repos incorrectly or ...
        //either ways this fixes duplicates
        .map { it.distinctBy { it.id } }
        .doOnSuccess { repoDao.insert(it) }

    private fun fetchCachedOrdered() = Single.fromCallable {
        when (Config.get<Int>(Config.Key.REPO_ORDER)) {
            Config.Value.ORDER_DATE -> repoDao.fetchAll()
            Config.Value.ORDER_NAME -> repoDao.fetchAllOrderByName()
            else -> repoDao.fetchAll()
        }
    }

    fun fetchInstalledModules() = Single.fromCallable { Utils.loadModulesLeanback() }
        .map { it.values.toList() }

    fun fetchInstallFile(module: String) = apiRaw
        .fetchFile(module, FILE_INSTALL_SH)
        .map { it.writeToFile(context, FILE_INSTALL_SH) }

    fun fetchReadme(module: String) = apiRaw
        .fetchFile(module, FILE_README_MD)
        .map { it.writeToString() }

    fun fetchConfig(module: String) = apiRaw
        .fetchFile(module, FILE_CONFIG_SH)
        .map { it.writeToFile(context, FILE_CONFIG_SH) }

    fun fetchInstallZip(module: String) = apiWeb
        .fetchModuleZip(module)
        .map { it.writeToFile(context, FILE_INSTALL_ZIP) }

    fun fetchInstaller() = apiRaw
        .fetchModuleInstaller()
        .map { it.writeToFile(context, FILE_MODULE_INSTALLER_SH) }

    fun deleteAllCached() = Completable.fromCallable { repoDao.deleteAll() }


    private fun fetchProperties(module: String, lastChanged: Long) = apiRaw
        .fetchFile(module, "module.prop")
        .map { it.toRepository(lastChanged) }

    private fun fetchAllRepos(page: Int = 0): Single<List<GithubRepo>> = api.fetchRepos(page)
        .flatMap {
            if (it.size == GithubApiServices.REPOS_PER_PAGE) {
                fetchAllRepos(page + 1).map { newList -> it + newList }
            } else {
                Single.just(it)
            }
        }

    companion object {
        const val FILE_INSTALL_SH = "install.sh"
        const val FILE_README_MD = "README.md"
        const val FILE_CONFIG_SH = "config.sh"
        const val FILE_INSTALL_ZIP = "install.zip"
        const val FILE_MODULE_INSTALLER_SH = "module_installer.sh"
    }

}