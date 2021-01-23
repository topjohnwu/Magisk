package com.topjohnwu.magisk.di

import android.content.Context
import androidx.preference.PreferenceManager
import com.topjohnwu.magisk.core.AssetHack
import com.topjohnwu.magisk.ktx.deviceProtectedContext
import org.koin.core.qualifier.named
import org.koin.dsl.module

val SUTimeout = named("su_timeout")
val Protected = named("protected")

val applicationModule = module {
    factory { AssetHack.resource }
    factory { get<Context>().packageManager }
    factory(Protected) { get<Context>().deviceProtectedContext }
    single(SUTimeout) { get<Context>(Protected).getSharedPreferences("su_timeout", 0) }
    single { PreferenceManager.getDefaultSharedPreferences(get(Protected)) }
}
