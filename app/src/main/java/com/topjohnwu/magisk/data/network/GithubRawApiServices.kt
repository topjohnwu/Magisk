package com.topjohnwu.magisk.data.network

import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.model.entity.UpdateInfo
import io.reactivex.Single
import okhttp3.ResponseBody
import retrofit2.http.GET
import retrofit2.http.Path
import retrofit2.http.Streaming
import retrofit2.http.Url


interface GithubRawApiServices {

    //region topjohnwu/magisk_files

    @GET("$MAGISK_FILES/master/stable.json")
    fun fetchStableUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/master/beta.json")
    fun fetchBetaUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/master/canary_builds/release.json")
    fun fetchCanaryUpdate(): Single<UpdateInfo>

    @GET("$MAGISK_FILES/master/canary_builds/canary.json")
    fun fetchCanaryDebugUpdate(): Single<UpdateInfo>

    @GET
    fun fetchCustomUpdate(@Url url: String): Single<UpdateInfo>

    @GET("$MAGISK_FILES/{$REVISION}/snet.apk")
    @Streaming
    fun fetchSafetynet(@Path(REVISION) revision: String = Const.SNET_REVISION): Single<ResponseBody>

    @GET("$MAGISK_FILES/{$REVISION}/bootctl")
    @Streaming
    fun fetchBootctl(@Path(REVISION) revision: String = Const.BOOTCTL_REVISION): Single<ResponseBody>

    //endregion

    /**
     * This method shall be used exclusively for fetching files from urls from previous requests.
     * Him, who uses it in a wrong way, shall die in an eternal flame.
     * */
    @GET
    @Streaming
    fun fetchFile(@Url url: String): Single<ResponseBody>


    companion object {
        private const val REVISION = "revision"
        private const val MODULE = "module"
        private const val FILE = "file"


        private const val MAGISK_FILES = "topjohnwu/magisk_files"
        private const val MAGISK_MASTER = "topjohnwu/Magisk/master"
        private const val MAGISK_MODULES = "Magisk-Modules-Repo"
    }

}