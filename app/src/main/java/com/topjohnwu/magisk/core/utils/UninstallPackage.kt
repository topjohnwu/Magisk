package com.topjohnwu.magisk.core.utils

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import androidx.activity.result.contract.ActivityResultContract

class UninstallPackage : ActivityResultContract<String, Boolean>() {

    @Suppress("DEPRECATION")
    override fun createIntent(context: Context, input: String): Intent {
        val uri = Uri.Builder().scheme("package").opaquePart(input).build()
        val intent = Intent(Intent.ACTION_UNINSTALL_PACKAGE, uri)
        intent.putExtra(Intent.EXTRA_RETURN_RESULT, true)
        return intent
    }

    override fun parseResult(resultCode: Int, intent: Intent?) =
        resultCode == Activity.RESULT_OK
}
