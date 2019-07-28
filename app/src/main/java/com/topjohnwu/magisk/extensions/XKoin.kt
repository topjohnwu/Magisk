package com.topjohnwu.magisk.extensions

import org.koin.core.context.GlobalContext
import org.koin.core.parameter.ParametersDefinition
import org.koin.core.qualifier.Qualifier

fun getKoin() = GlobalContext.get().koin

inline fun <reified T> inject(
    qualifier: Qualifier? = null,
    noinline parameters: ParametersDefinition? = null
) = lazy { get<T>(qualifier, parameters) }

inline fun <reified T> get(
    qualifier: Qualifier? = null,
    noinline parameters: ParametersDefinition? = null
): T = getKoin().get(qualifier, parameters)