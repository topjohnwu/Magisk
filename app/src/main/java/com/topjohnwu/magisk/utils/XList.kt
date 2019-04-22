package com.topjohnwu.magisk.utils

fun <T> MutableList<T>.update(newList: List<T>) {
    clear()
    addAll(newList)
}