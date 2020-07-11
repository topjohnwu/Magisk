package com.topjohnwu.magisk.ktx

import android.content.res.Resources
import kotlin.math.ceil
import kotlin.math.roundToInt

fun Int.toDp(): Int = ceil(this / Resources.getSystem().displayMetrics.density).roundToInt()

fun Int.toPx(): Int = (this * Resources.getSystem().displayMetrics.density).roundToInt()
