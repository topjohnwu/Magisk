package com.topjohnwu.magisk.extensions

import io.reactivex.Single
import io.reactivex.functions.BiFunction

fun <T : Any> T.toSingle() = Single.just(this)

fun <T1, T2, R> zip(t1: Single<T1>, t2: Single<T2>, zipper: (T1, T2) -> R) =
    Single.zip(t1, t2, BiFunction<T1, T2, R> { rt1, rt2 -> zipper(rt1, rt2) })