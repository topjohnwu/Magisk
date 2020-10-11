package com.topjohnwu.magisk.core

import android.content.Context
import android.content.pm.ProviderInfo
import android.os.Bundle
import com.topjohnwu.magisk.FileProvider
import com.topjohnwu.magisk.core.su.SuCallbackHandler

open class Provider : FileProvider() {

    override fun attachInfo(context: Context, info: ProviderInfo?) {
        super.attachInfo(context.wrap(), info)
    }

    override fun call(method: String, arg: String?, extras: Bundle?): Bundle? {
        SuCallbackHandler(context!!, method, extras)
        return Bundle.EMPTY
    }
}
