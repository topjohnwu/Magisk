package com.topjohnwu.magisk.di

import com.squareup.moshi.JsonAdapter
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.data.network.GithubRawApiServices
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import org.koin.dsl.module
import retrofit2.CallAdapter
import retrofit2.Converter
import retrofit2.Retrofit
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory
import retrofit2.converter.moshi.MoshiConverterFactory
import se.ansman.kotshi.KotshiJsonAdapterFactory

val networkingModule = module {
    single { createOkHttpClient() }
    single { createConverterFactory() }
    single { createCallAdapterFactory() }
    single { createRetrofit(get(), get(), get()) }
    single { createApiService<GithubRawApiServices>(get(), Const.Url.GITHUB_RAW_API_URL) }
}

fun createOkHttpClient(): OkHttpClient {
    val builder = OkHttpClient.Builder()

    if (BuildConfig.DEBUG) {
        val httpLoggingInterceptor = HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.BODY
        }
        builder.addInterceptor(httpLoggingInterceptor)
    }

    return builder.build()
}

fun createConverterFactory(): Converter.Factory {
    val moshi = Moshi.Builder()
            .add(JsonAdapterFactory.INSTANCE)
            .build()
    return MoshiConverterFactory.create(moshi)
}

fun createCallAdapterFactory(): CallAdapter.Factory {
    return RxJava2CallAdapterFactory.create()
}

fun createRetrofit(
    okHttpClient: OkHttpClient,
    converterFactory: Converter.Factory,
    callAdapterFactory: CallAdapter.Factory
): Retrofit.Builder {
    return Retrofit.Builder()
        .addConverterFactory(converterFactory)
        .addCallAdapterFactory(callAdapterFactory)
        .client(okHttpClient)
}

@KotshiJsonAdapterFactory
abstract class JsonAdapterFactory : JsonAdapter.Factory {
    companion object {
        val INSTANCE: JsonAdapterFactory = KotshiJsonAdapterFactory
    }
}

inline fun <reified T> createApiService(retrofitBuilder: Retrofit.Builder, baseUrl: String): T {
    return retrofitBuilder
        .baseUrl(baseUrl)
        .build()
        .create(T::class.java)
}