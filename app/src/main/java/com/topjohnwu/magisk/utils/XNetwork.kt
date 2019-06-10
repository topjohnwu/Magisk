package com.topjohnwu.magisk.utils

import android.content.Context
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
