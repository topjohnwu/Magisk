package com.topjohnwu.magisk

import android.content.Context
import android.content.SharedPreferences
import androidx.annotation.AnyThread
import androidx.annotation.WorkerThread
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.data.repository.SettingRepository
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.utils.inject

object ConfigLeanback {

    @JvmStatic
    val protectedContext: Context by inject(Protected)
    @JvmStatic
    val prefs: SharedPreferences by inject()

    private val settingRepo: SettingRepository by inject()
    private val stringRepo: StringRepository by inject()

    @JvmStatic
    @AnyThread
    fun put(key: String, value: Int) {
        settingRepo.put(key, value).subscribeK()
    }

    @JvmStatic
    @WorkerThread
    fun get(key: String, defaultValue: Int): Int =
        settingRepo.fetch(key, defaultValue).blockingGet()

    @JvmStatic
    @AnyThread
    fun put(key: String, value: String?) {
        val task = value?.let { stringRepo.put(key, it) } ?: stringRepo.delete(key)
        task.subscribeK()
    }

    @JvmStatic
    @WorkerThread
    fun get(key: String, defaultValue: String?): String =
        stringRepo.fetch(key, defaultValue.orEmpty()).blockingGet()

    @JvmStatic
    @AnyThread
    fun delete(key: String) {
        settingRepo.delete(key).subscribeK()
    }

}