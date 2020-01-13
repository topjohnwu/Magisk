package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.data.network.GithubRawServices

class StringRepository(
    private val api: GithubRawServices
) {

    fun getString(url: String) = api.fetchString(url)

    fun getMetadata(repo: Repo) = api.fetchModuleInfo(repo.id, "module.prop")

    fun getReadme(repo: Repo) = api.fetchModuleInfo(repo.id, "README.md")
}
