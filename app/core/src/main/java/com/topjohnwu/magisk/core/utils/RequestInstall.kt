package com.topjohnwu.magisk.core.utils

import android.annotation.TargetApi
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.Settings
import androidx.activity.result.contract.ActivityResultContract

class RequestInstall : ActivityResultContract<Unit, Boolean>() {

    @TargetApi(26)
    override fun createIntent(context: Context, input: Unit): Intent {
        // This will only be called on API 26+
        return Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES)
            .setData(Uri.parse("package:${context.packageName}"))
    }

    override fun parseResult(resultCode: Int, intent: Intent?) =
        resultCode == Activity.RESULT_OK

    override fun getSynchronousResult(
        context: Context,
        input: Unit
    ): SynchronousResult<Boolean>? {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O)
            return SynchronousResult(true)
        if (context.packageManager.canRequestPackageInstalls())
            return SynchronousResult(true)
        return null
    }
}
