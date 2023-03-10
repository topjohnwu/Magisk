package com.topjohnwu.magisk.databinding

import kotlinx.coroutines.CoroutineScope

fun <T : AnyDiffRvItem> diffListOf() =
    DiffObservableList(DiffRvItem.callback<T>())

fun <T : AnyDiffRvItem> filterableListOf(scope: CoroutineScope) =
    FilterableDiffObservableList(DiffRvItem.callback<T>(), scope)
