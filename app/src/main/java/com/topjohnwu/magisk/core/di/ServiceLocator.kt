package com.topjohnwu.magisk.core.di

import android.annotation.SuppressLint
import android.content.Context
import android.text.method.LinkMovementMethod
import androidx.room.Room
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.data.SuLogDatabase
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.data.magiskdb.SettingsDao
import com.topjohnwu.magisk.core.data.magiskdb.StringDao
import com.topjohnwu.magisk.core.ktx.deviceProtectedContext
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.utils.BiometricHelper
import io.noties.markwon.Markwon
import io.noties.markwon.utils.NoCopySpannableFactory

val AppContext: Context inline get() = ServiceLocator.context

@SuppressLint("StaticFieldLeak")
object ServiceLocator {

    lateinit var context: Context
    val deContext by lazy { context.deviceProtectedContext }
    val timeoutPrefs by lazy { deContext.getSharedPreferences("su_timeout", 0) }
    val biometrics by lazy { BiometricHelper(context) }

    // Database
    val policyDB = PolicyDao()
    val settingsDB = SettingsDao()
    val stringDB = StringDao()
    val sulogDB by lazy { createSuLogDatabase(deContext).suLogDao() }
    val logRepo by lazy { LogRepository(sulogDB) }

    // Networking
    val okhttp by lazy { createOkHttpClient(context) }
    val retrofit by lazy { createRetrofit(okhttp) }
    val markwon by lazy { createMarkwon(context) }
    val networkService by lazy {
        NetworkService(
            createApiService(retrofit, Const.Url.GITHUB_PAGE_URL),
            createApiService(retrofit, Const.Url.GITHUB_RAW_URL),
        )
    }
}

private fun createSuLogDatabase(context: Context) =
    Room.databaseBuilder(context, SuLogDatabase::class.java, "sulogs.db")
        .addMigrations(SuLogDatabase.MIGRATION_1_2)
        .fallbackToDestructiveMigration()
        .build()

private fun createMarkwon(context: Context) =
    Markwon.builder(context).textSetter { textView, spanned, bufferType, onComplete ->
        textView.apply {
            movementMethod = LinkMovementMethod.getInstance()
            setSpannableFactory(NoCopySpannableFactory.getInstance())
            setText(spanned, bufferType)
            onComplete.run()
        }
    }.build()
