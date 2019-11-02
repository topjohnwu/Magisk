package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.model.entity.module.Repo

class StringRepository(
    private val api: GithubRawServices
) {

    fun getString(url: String) = api.fetchString(url)

    fun getMetadata(repo: Repo) = api.fetchModuleInfo(repo.id, "module.prop")

    fun getReadme(repo: Repo) = api.fetchModuleInfo(repo.id, "README.md")
}