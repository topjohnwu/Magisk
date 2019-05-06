package com.topjohnwu.magisk.di

import com.squareup.moshi.Moshi
import com.squareup.moshi.kotlin.reflect.KotlinJsonAdapterFactory
import com.topjohnwu.magisk.Constants
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubRawApiServices
import com.topjohnwu.magisk.data.network.GithubServices
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import org.koin.dsl.module
import retrofit2.CallAdapter
import retrofit2.Converter
import retrofit2.Retrofit
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory
import retrofit2.converter.moshi.MoshiConverterFactory

val networkingModule = module {
    single { createOkHttpClient() }

    single { createConverterFactory() }
    single { createCallAdapterFactory() }

    single { createRetrofit(get(), get(), get()) }

    single { createApiService<GithubServices>(get(), Constants.GITHUB_URL) }
    single { createApiService<GithubApiServices>(get(), Constants.GITHUB_API_URL) }
    single { createApiService<GithubRawApiServices>(get(), Constants.GITHUB_RAW_API_URL) }
}

fun createOkHttpClient(): OkHttpClient {
    val httpLoggingInterceptor = HttpLoggingInterceptor().apply {
        level = HttpLoggingInterceptor.Level.BODY
    }

    return OkHttpClient.Builder()
        .addInterceptor(httpLoggingInterceptor)
        .build()
}

fun createConverterFactory(): Converter.Factory {
    val moshi = Moshi.Builder()
        .add(KotlinJsonAdapterFactory())
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

inline fun <reified T> createApiService(retrofitBuilder: Retrofit.Builder, baseUrl: String): T {
    return retrofitBuilder
        .baseUrl(baseUrl)
        .build()
        .create(T::class.java)
}