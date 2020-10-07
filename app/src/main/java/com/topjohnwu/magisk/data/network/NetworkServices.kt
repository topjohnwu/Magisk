package com.topjohnwu.magisk.data.network

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.BranchInfo
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.core.tasks.GithubRepoInfo
import okhttp3.ResponseBody
import retrofit2.Response
import retrofit2.http.*

private const val REVISION = "revision"
private const val MODULE = "module"
private const val FILE = "file"
private const val IF_NONE_MATCH = "If-None-Match"
private const val BRANCH = "branch"
private const val REPO = "repo"

const val MAGISK_FILES = "topjohnwu/magisk_files"
const val MAGISK_MAIN = "topjohnwu/Magisk"
private const val MAGISK_MODULES = "Magick-Modules-Repo"

interface GithubPageServices {

    @GET("stable.json")
    suspend fun fetchStableUpdate(): UpdateInfo

    @GET("beta.json")
    suspend fun fetchBetaUpdate(): UpdateInfo
}

interface JSDelivrServices {

    @GET("$MAGISK_FILES@{$REVISION}/snet")
    @Streaming
    suspend fun fetchSafetynet(@Path(REVISION) revision: String = Const.SNET_REVISION): ResponseBody

    @GET("$MAGISK_FILES@{$REVISION}/bootctl")
    @Streaming
    suspend fun fetchBootctl(@Path(REVISION) revision: String = Const.BOOTCTL_REVISION): ResponseBody

    @GET("$MAGISK_FILES@{$REVISION}/canary.json")
    suspend fun fetchCanaryUpdate(@Path(REVISION) revision: String): UpdateInfo

    @GET("$MAGISK_MAIN@{$REVISION}/scripts/module_installer.sh")
    @Streaming
    suspend fun fetchInstaller(@Path(REVISION) revision: String): ResponseBody
}

interface GithubRawServices {

    @GET
    suspend fun fetchCustomUpdate(@Url url: String): UpdateInfo

    @GET("$MAGISK_MODULES/{$MODULE}/master/{$FILE}")
    suspend fun fetchModuleFile(@Path(MODULE) id: String, @Path(FILE) file: String): String

    @GET
    @Streaming
    suspend fun fetchFile(@Url url: String): ResponseBody

    @GET
    suspend fun fetchString(@Url url: String): String

}

interface GithubApiServices {

    @GET("users/$MAGISK_MODULES/repos")
    @Headers("Accept: application/vnd.github.v3+json")
    suspend fun fetchRepos(
        @Query("page") page: Int,
        @Header(IF_NONE_MATCH) etag: String,
        @Query("sort") sort: String = "pushed",
        @Query("per_page") count: Int = 100
    ): Response<List<GithubRepoInfo>>

    @GET("repos/{$REPO}/branches/{$BRANCH}")
    @Headers("Accept: application/vnd.github.v3+json")
    suspend fun fetchBranch(
        @Path(REPO, encoded = true) repo: String,
        @Path(BRANCH) branch: String
    ): BranchInfo
}

