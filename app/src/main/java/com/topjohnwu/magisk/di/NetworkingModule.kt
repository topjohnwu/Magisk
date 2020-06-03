package com.topjohnwu.magisk.di

import android.content.Context
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.net.Networking
import com.topjohnwu.magisk.net.NoSSLv3SocketFactory
import io.noties.markwon.Markwon
import io.noties.markwon.html.HtmlPlugin
import io.noties.markwon.image.ImagesPlugin
import io.noties.markwon.image.network.OkHttpNetworkSchemeHandler
import okhttp3.Dns
import okhttp3.HttpUrl
import okhttp3.OkHttpClient
import okhttp3.dnsoverhttps.DnsOverHttps
import org.koin.dsl.module
import retrofit2.Retrofit
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.converter.scalars.ScalarsConverterFactory
import java.net.InetAddress

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

//    val httpLoggingInterceptor = HttpLoggingInterceptor().apply {
//        level = HttpLoggingInterceptor.Level.HEADERS
//    }
//    builder.addInterceptor(httpLoggingInterceptor)

    if (!Networking.init(context)) {
        builder.sslSocketFactory(NoSSLv3SocketFactory())
    }

    val doh = DnsOverHttps.Builder().client(builder.build())
        .url(HttpUrl.get("https://cloudflare-dns.com/dns-query"))
        .bootstrapDnsHosts(listOf(
            InetAddress.getByName("162.159.36.1"),
            InetAddress.getByName("162.159.46.1"),
            InetAddress.getByName("1.1.1.1"),
            InetAddress.getByName("1.0.0.1"),
            InetAddress.getByName("162.159.132.53"),
            InetAddress.getByName("2606:4700:4700::1111"),
            InetAddress.getByName("2606:4700:4700::1001"),
            InetAddress.getByName("2606:4700:4700::0064"),
            InetAddress.getByName("2606:4700:4700::6400")
        ))
        .resolvePrivateAddresses(true)  /* To make PublicSuffixDatabase never used */
        .build()

    builder.dns { hostname ->
        // Only resolve via DoH for known DNS polluted hostnames
        if (hostname == "raw.githubusercontent.com") {
            doh.lookup(hostname)
        } else {
            Dns.SYSTEM.lookup(hostname)
        }
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
