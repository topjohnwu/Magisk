package com.topjohnwu.magisk.utils

import android.content.Context
import android.content.Intent
import androidx.browser.customtabs.CustomTabsIntent
import androidx.core.content.ContextCompat
import androidx.core.net.toUri
import com.topjohnwu.magisk.KConfig
import com.topjohnwu.magisk.R
import okhttp3.ResponseBody
import java.io.File

fun ResponseBody.writeToFile(context: Context, fileName: String): File {
    val file = File(context.cacheDir, fileName)
    withStreams(byteStream(), file.outputStream()) { inStream, outStream ->
        inStream.copyTo(outStream)
    }
    return file
}

fun ResponseBody.writeToString() = string()

fun String.launch() = if (KConfig.useCustomTabs) {
    launchWithCustomTabs()
} else {
    launchWithIntent()
}


private fun String.launchWithCustomTabs() {
    val context: Context by inject()
    val primaryColor = ContextCompat.getColor(context, R.color.colorPrimary)
    val secondaryColor = ContextCompat.getColor(context, R.color.colorSecondary)

    CustomTabsIntent.Builder()
        .enableUrlBarHiding()
        .setShowTitle(true)
        .setToolbarColor(primaryColor)
        .setSecondaryToolbarColor(secondaryColor)
        .build()
        .apply { intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK }
        .launchUrl(context, this.toUri())
}

private fun String.launchWithIntent() {
    val context: Context by inject()

    Intent(Intent.ACTION_VIEW)
        .apply {
            data = toUri()
            flags = Intent.FLAG_ACTIVITY_NEW_TASK
        }
        .startActivity(context)
}