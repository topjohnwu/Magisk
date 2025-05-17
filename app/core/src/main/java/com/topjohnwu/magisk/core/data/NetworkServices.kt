package com.topjohnwu.magisk.core.data

import com.topjohnwu.magisk.core.model.ModuleJson
import com.topjohnwu.magisk.core.model.Release
import com.topjohnwu.magisk.core.model.UpdateInfo
import okhttp3.ResponseBody
import retrofit2.http.GET
import retrofit2.http.Headers
import retrofit2.http.Path
import retrofit2.http.Query
import retrofit2.http.Streaming
import retrofit2.http.Url

interface RawServices {

    @GET
    @Streaming
    suspend fun fetchFile(@Url url: String): ResponseBody

    @GET
    suspend fun fetchString(@Url url: String): String

    @GET
    suspend fun fetchModuleJson(@Url url: String): ModuleJson

    @GET
    suspend fun fetchUpdateJSON(@Url url: String): UpdateInfo

}

interface GithubApiServices {

    @GET("/repos/{owner}/{repo}/releases")
    @Headers("Accept: application/vnd.github+json")
    suspend fun fetchRelease(
        @Path("owner") owner: String = "topjohnwu",
        @Path("repo") repo: String = "Magisk",
        @Query("per_page") per: Int = 10,
        @Query("page") page: Int = 1,
    ): List<Release>

    @GET("/repos/{owner}/{repo}/releases/latest")
    @Headers("Accept: application/vnd.github+json")
    suspend fun fetchLatestRelease(
        @Path("owner") owner: String = "topjohnwu",
        @Path("repo") repo: String = "Magisk",
    ): Release
}
