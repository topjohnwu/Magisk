package com.topjohnwu.magisk.data.network

import io.reactivex.Single
import okhttp3.ResponseBody
import retrofit2.http.GET
import retrofit2.http.Path
import retrofit2.http.Streaming


interface GithubServices {

    @GET("Magisk-Modules-Repo/{$MODULE}/archive/master.zip")
    @Streaming
    fun fetchModuleZip(@Path(MODULE) module: String): Single<ResponseBody>


    companion object {
        private const val MODULE = "module"
    }

}