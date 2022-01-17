package com.topjohnwu.magisk.ktx

import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flatMapMerge
import kotlinx.coroutines.flow.flow

inline fun <T, R> Flow<T>.concurrentMap(crossinline transform: suspend (T) -> R): Flow<R> {
    return flatMapMerge { value ->
        flow { emit(transform(value)) }
    }
}
