package com.topjohnwu.magisk.di

import android.content.Context
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.ProviderInstaller
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ktx.precomputedText
import com.topjohnwu.magisk.utils.MarkwonImagePlugin
import io.noties.markwon.Markwon
import io.noties.markwon.html.HtmlPlugin
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.converter.scalars.ScalarsConverterFactory

@Suppress("DEPRECATION")
fun createOkHttpClient(context: Context): OkHttpClient {
    val builder = OkHttpClient.Builder()

    if (BuildConfig.DEBUG) {
        builder.addInterceptor(HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.BASIC
        })
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

fun createMarkwon(context: Context, okHttpClient: OkHttpClient): Markwon {
    return Markwon.builder(context)
        .textSetter { textView, spanned, _, onComplete ->
            textView.tag = onComplete
            textView.precomputedText = spanned
        }
        .usePlugin(HtmlPlugin.create())
        .usePlugin(MarkwonImagePlugin(okHttpClient))
        .build()
}
