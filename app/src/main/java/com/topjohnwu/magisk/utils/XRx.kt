package com.topjohnwu.magisk.utils

import io.reactivex.Single

fun <T : Any> T.toSingle() = Single.just(this)