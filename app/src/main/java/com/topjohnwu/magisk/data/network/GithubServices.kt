package com.topjohnwu.magisk.data.network

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.core.tasks.GithubRepoInfo
import io.reactivex.Flowable
import io.reactivex.Single
import okhttp3.ResponseBody
import retrofit2.adapter.rxjava2.Result
import retrofit2.http.*

interface GithubRawServices {

    //region topjohnwu/magisk_files

    @GET("$MAGISK_FILES/master/stable.json")
    fun fetchStableUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/master/beta.json")
    fun fetchBetaUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/canary/release.json")
    fun fetchCanaryUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/canary/debug.json")
    fun fetchCanaryDebugUpdate(): Single<UpdateInfo>

    @GET
    fun fetchCustomUpdate(@Url url: String): Single<UpdateInfo>

    @GET("$MAGISK_FILES/{$REVISION}/snet.jar")
    @Streaming
    fun fetchSafetynet(@Path(REVISION) revision: String = Const.SNET_REVISION): Single<ResponseBody>

    @GET("$MAGISK_FILES/{$REVISION}/bootctl")
    @Streaming
    fun fetchBootctl(@Path(REVISION) revision: String = Const.BOOTCTL_REVISION): Single<ResponseBody>

    @GET("$MAGISK_MASTER/scripts/module_installer.sh")
    @Streaming
    fun fetchInstaller(): Single<ResponseBody>

    @GET("$MAGISK_MODULES/{$MODULE}/master/{$FILE}")
    fun fetchModuleInfo(@Path(MODULE) id: String, @Path(FILE) file: String): Single<String>

    //endregion

    /**
     * This method shall be used exclusively for fetching files from urls from previous requests.
     * Him, who uses it in a wrong way, shall die in an eternal flame.
     * */
    @GET
    @Streaming
    fun fetchFile(@Url url: String): Single<ResponseBody>

    @GET
    fun fetchString(@Url url: String): Single<String>


    companion object {
        private const val REVISION = "revision"
        private const val MODULE = "module"
        private const val FILE = "file"


        private const val MAGISK_FILES = "topjohnwu/magisk_files"
        private const val MAGISK_MASTER = "topjohnwu/Magisk/master"
        private const val MAGISK_MODULES = "Magisk-Modules-Repo"
    }

}

interface GithubApiServices {

    @GET("repos")
    fun fetchRepos(@Query("page") page: Int,
                   @Header(Const.Key.IF_NONE_MATCH) etag: String,
                   @Query("sort") sort: String = "pushed",
                   @Query("per_page") count: Int = 100): Flowable<Result<List<GithubRepoInfo>>>

}
