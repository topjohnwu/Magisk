package com.topjohnwu.magisk.extensions

import android.os.Handler
import android.os.Looper

fun ui(body: () -> Unit) = Handler(Looper.getMainLooper()).post(body)