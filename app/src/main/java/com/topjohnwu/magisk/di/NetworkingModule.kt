package com.topjohnwu.magisk.di

import android.content.Context
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.net.Networking
import com.topjohnwu.magisk.net.NoSSLv3SocketFactory
import io.noties.markwon.Markwon
import io.noties.markwon.html.HtmlPlugin
import io.noties.markwon.image.ImagesPlugin
import io.noties.markwon.image.network.OkHttpNetworkSchemeHandler
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import org.koin.dsl.module
import retrofit2.Retrofit
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.converter.scalars.ScalarsConverterFactory

val networkingModule = module {
    single { createOkHttpClient(get()) }
    single { createRetrofit(get()) }
    single { createApiService<GithubRawServices>(get(), Const.Url.GITHUB_RAW_URL) }
    single { createApiService<GithubApiServices>(get(), Const.Url.GITHUB_API_URL) }
    single { createMarkwon(get(), get()) }
}

@Suppress("DEPRECATION")
fun createOkHttpClient(context: Context): OkHttpClient {
    val builder = OkHttpClient.Builder()

    if (BuildConfig.DEBUG) {
        val httpLoggingInterceptor = HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.HEADERS
        }
        builder.addInterceptor(httpLoggingInterceptor)
    }

    if (!Networking.init(context)) {
        builder.sslSocketFactory(NoSSLv3SocketFactory())
    }

    return builder.build()
}

fun createMoshiConverterFactory(): MoshiConverterFactory {
    val moshi = Moshi.Builder().build()
    return MoshiConverterFactory.create(moshi)
}

fun createRetrofit(okHttpClient: OkHttpClient): Retrofit.Builder {
    return Retrofit.Builder()
        .addConverterFactory(ScalarsConverterFactory.create())
        .addConverterFactory(createMoshiConverterFactory())
        .addCallAdapterFactory(RxJava2CallAdapterFactory.create())
        .client(okHttpClient)
}

inline fun <reified T> createApiService(retrofitBuilder: Retrofit.Builder, baseUrl: String): T {
    return retrofitBuilder
        .baseUrl(baseUrl)
        .build()
        .create(T::class.java)
}

fun createMarkwon(context: Context, okHttpClient: OkHttpClient): Markwon {
    return Markwon.builder(context)
        .usePlugin(HtmlPlugin.create())
        .usePlugin(ImagesPlugin.create {
            it.addSchemeHandler(OkHttpNetworkSchemeHandler.create(okHttpClient))
        })
        .build()
}
