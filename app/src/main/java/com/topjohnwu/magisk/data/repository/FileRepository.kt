package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.network.GithubRawApiServices

class FileRepository(
    private val api: GithubRawApiServices
) {

    fun downloadFile(url: String) = api.fetchFile(url)

}