package com.topjohnwu.magisk.core.di

import android.content.Context
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.ProviderInstaller
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.utils.currentLocale
import okhttp3.Cache
import okhttp3.ConnectionSpec
import okhttp3.Dns
import okhttp3.HttpUrl.Companion.toHttpUrl
import okhttp3.OkHttpClient
import okhttp3.dnsoverhttps.DnsOverHttps
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.converter.scalars.ScalarsConverterFactory
import java.io.File
import java.net.InetAddress
import java.net.UnknownHostException

private class DnsResolver(client: OkHttpClient) : Dns {

    private val doh by lazy {
        DnsOverHttps.Builder().client(client)
            .url("https://101.101.101.101/dns-query".toHttpUrl())
            .resolvePrivateAddresses(true)  /* To make PublicSuffixDatabase never used */
            .build()
    }

    override fun lookup(hostname: String): List<InetAddress> {
        if (Config.doh) {
            try {
                return doh.lookup(hostname)
            } catch (e: UnknownHostException) {}
        }
        return Dns.SYSTEM.lookup(hostname)
    }
}


fun createOkHttpClient(context: Context): OkHttpClient {
    val appCache = Cache(File(context.cacheDir, "okhttp"), 10 * 1024 * 1024)
    val builder = OkHttpClient.Builder().cache(appCache)

    if (BuildConfig.DEBUG) {
        builder.addInterceptor(HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.BASIC
        })
    } else {
        builder.connectionSpecs(listOf(ConnectionSpec.MODERN_TLS))
    }

    builder.dns(DnsResolver(builder.build()))

    builder.addInterceptor { chain ->
        val request = chain.request().newBuilder()
        request.header("User-Agent", "Magisk/${BuildConfig.VERSION_CODE}")
        request.header("Accept-Language", currentLocale.toLanguageTag())
        chain.proceed(request.build())
    }

    if (!ProviderInstaller.install(context)) {
        Info.hasGMS = false
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
        .client(okHttpClient)
}

inline fun <reified T> createApiService(retrofitBuilder: Retrofit.Builder, baseUrl: String): T {
    return retrofitBuilder
        .baseUrl(baseUrl)
        .build()
        .create(T::class.java)
}
