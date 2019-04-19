package com.topjohnwu.magisk.utils

import org.koin.core.context.GlobalContext
import org.koin.core.parameter.ParametersDefinition
import org.koin.core.qualifier.Qualifier
import org.koin.core.scope.Scope

fun getKoin() = GlobalContext.get().koin

inline fun <reified T : Any> inject(
    qualifier: Qualifier? = null,
    scope: Scope? = null,
    noinline parameters: ParametersDefinition? = null
) = lazy { get<T>(qualifier, scope, parameters) }

inline fun <reified T : Any> get(
    qualifier: Qualifier? = null,
    scope: Scope? = null,
    noinline parameters: ParametersDefinition? = null
): T = getKoin().get(qualifier, scope, parameters)