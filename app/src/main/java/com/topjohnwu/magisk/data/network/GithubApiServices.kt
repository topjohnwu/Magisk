package com.topjohnwu.magisk.data.network

import com.topjohnwu.magisk.model.entity.GithubRepo
import io.reactivex.Single
import retrofit2.http.GET
import retrofit2.http.Query


interface GithubApiServices {

    @GET("users/Magisk-Modules-Repo/repos")
    fun fetchRepos(
        @Query("page") page: Int,
        @Query("per_page") count: Int = REPOS_PER_PAGE,
        @Query("sort") sortOrder: String = "pushed"
    ): Single<List<GithubRepo>>

    companion object {
        const val REPOS_PER_PAGE = 100
    }

}