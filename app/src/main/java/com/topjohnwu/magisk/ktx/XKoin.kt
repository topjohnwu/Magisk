package com.topjohnwu.magisk.ktx

import org.koin.core.context.KoinContextHandler
import org.koin.core.parameter.ParametersDefinition
import org.koin.core.qualifier.Qualifier

fun getKoin() = KoinContextHandler.get()

inline fun <reified T> inject(
    qualifier: Qualifier? = null,
    noinline parameters: ParametersDefinition? = null
) = lazy { get<T>(qualifier, parameters) }

inline fun <reified T> get(
    qualifier: Qualifier? = null,
    noinline parameters: ParametersDefinition? = null
): T = getKoin().get(qualifier, parameters)
