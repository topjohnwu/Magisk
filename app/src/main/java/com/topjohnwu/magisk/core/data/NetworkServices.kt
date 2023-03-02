package com.topjohnwu.magisk.core.data

import com.topjohnwu.magisk.core.model.BranchInfo
import com.topjohnwu.magisk.core.model.ModuleJson
import com.topjohnwu.magisk.core.model.UpdateInfo
import okhttp3.ResponseBody
import retrofit2.http.*

private const val BRANCH = "branch"
private const val REPO = "repo"
private const val FILE = "file"

interface GithubPageServices {

    @GET
    suspend fun fetchUpdateJSON(@Url file: String): UpdateInfo
}

interface RawServices {

    @GET
    @Streaming
    suspend fun fetchFile(@Url url: String): ResponseBody

    @GET
    suspend fun fetchString(@Url url: String): String

    @GET
    suspend fun fetchModuleJson(@Url url: String): ModuleJson

}

interface GithubApiServices {

    @GET("repos/{$REPO}/branches/{$BRANCH}")
    @Headers("Accept: application/vnd.github.v3+json")
    suspend fun fetchBranch(
        @Path(REPO, encoded = true) repo: String,
        @Path(BRANCH) branch: String
    ): BranchInfo
}
